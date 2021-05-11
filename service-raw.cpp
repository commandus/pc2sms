#include "service-raw.h"
#include <sstream>
#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>
#include "errlist.h"
#include "platform.h"
#include "utilstring.h"

#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

// #define DEBUG 1

static void rollback(PGconn *conn, PGresult *r) {
    r = PQexec(conn, "ROLLBACK");
    PQclear(r);
}

/**
 *
 * @param conn database connection
 * @param retid return watcher id
 * @return rights mask
 * 
 **/
static int32_t getWatcherRights
(
  PGconn *conn,
  uint64_t &retid,
  const gpstracker::Watcher &watcher
) {
#ifdef DEBUG  
  std::cerr << watcher.login() << "/" << watcher.password() << std::endl;
#endif
  const char *values[2] = { (char *) watcher.login().c_str(), watcher.password().c_str() };
  int lengths[2] = { (int) watcher.login().size(), (int) watcher.password().size() };
  int binary[2] = { 0, 0 };

  // 4- user
  PGresult *r = PQexecParams(conn, "SELECT id, rights \
    FROM watcher WHERE login = $1 and \"password\" = $2 LIMIT 1",
    2, NULL, values, lengths, binary, 0);
	int rows = PQntuples(r);
  int32_t rights = 0;
  if (rows > 0) {
    retid = strtoull(PQgetvalue(r, 0, 0), NULL, 10);
    rights = strtol(PQgetvalue(r, 0, 1), NULL, 10);
  }
  PQclear(r);
  return rights;
}

/**
 * 1- root
 * 2- group mgr
 * 4- user
 */
static bool hasAdminRights
(
  PGconn *conn,
  const gpstracker::Watcher &watcher
) {
  // prevent useless database call
  if (watcher.login().empty()) {
    return false;
  }
  uint64_t uid;
  return (getWatcherRights(conn, uid, watcher) & 1) != 0;
}

/**
 * 
 */
static int loadDevice(
  bool isAdmin,
  gpstracker::Device& device,
  PGconn *conn
) 
{
  uint64_t ndeviceid = htonll(device.id());

  const char *values[2] = { (const char *) &ndeviceid, (const char *) device.password().c_str() };
  int lengths[2] = { (int) sizeof(ndeviceid), (int) device.password().size() };
  int binary[2] = { 1, 0  };

  // get device.WARNING:  there is already a transaction in progress
  std::string q;
  if (isAdmin)
    q = "SELECT id, \"name\", color, imei, \"password\", tag FROM device \
    WHERE id = $1 LIMIT 1";
  else
    q = "SELECT id, \"name\", color, imei, \"password\", tag FROM device \
    WHERE id = $1 AND (\"password\" IS NULL OR \"password\" = '' OR \"password\" = $2)\
    LIMIT 1";
  PGresult *r = PQexecParams(conn, q.c_str(), isAdmin ? 1 : 2, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_TUPLES_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }
	int rows = PQntuples(r);
  if (rows > 0) {
    char *v = PQgetvalue(r, 0, 1);
    device.set_name(v);
    v = PQgetvalue(r, 0, 2);
    device.set_color(v);
    v = PQgetvalue(r, 0, 4);
    device.set_password(v);
    v = PQgetvalue(r, 0, 5);
    device.set_tag(strtol(v, NULL, 10));
  }
  PQclear(r);
  return rows > 0 ? TRACKER_OK : ERR_CODE_GET_DEVICE;
}

static int loadDevices(
  std::vector<gpstracker::Device> &devices,
  uint64_t watcherId,
  int32_t rights,
  size_t maxcount,
  size_t offset,
  PGconn *conn
) 
{
  uint64_t nwatcherid = htonll(watcherId);

  const char *values[1] = { (const char *) &nwatcherid };
  int lengths[1] = { (int) sizeof(nwatcherid) };
  int binary[1] = { 1 };

  bool isAdmin = (rights & 1) != 0;
  // get device.WARNING:  there is already a transaction in progress
  std::stringstream q;
  PGresult *r;
  if (isAdmin) {
    q << "SELECT id, \"name\", color, imei, \"password\", tag FROM device LIMIT "
      << maxcount << " OFFSET " << offset;
    r = PQexec(conn, q.str().c_str());
  } else {
    q << "SELECT id, \"name\", color, imei, \"password\", tag FROM device \
    WHERE id IN (SELECT deviceid FROM watcher2device WHERE watcherid = $1) LIMIT "
    << maxcount << " OFFSET " << offset;
    r = PQexecParams(conn, q.str().c_str(), 1, NULL, values, lengths, binary, 0);
  }
  if (PQresultStatus(r) != PGRES_TUPLES_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }
	int rows = PQntuples(r);
  for (int row = 0; row < rows; row++) {
    gpstracker::Device device;
    char *v = PQgetvalue(r, row, 0);
    device.set_id(strtoull(v, NULL, 10));
    v = PQgetvalue(r, row, 1);
    device.set_name(v);
    v = PQgetvalue(r, row, 2);
    device.set_color(v);
    v = PQgetvalue(r, row, 3);
    device.set_imei(v);
    v = PQgetvalue(r, row, 4);
    device.set_password(v);
    v = PQgetvalue(r, row, 5);
    device.set_tag(strtol(v, NULL, 10));
    devices.push_back(device);
  }
  PQclear(r);
  return rows > 0 ? TRACKER_OK : ERR_CODE_GET_DEVICE;
}

static int loadWatcher(
  gpstracker::Watcher &watcher,
  uint64_t watcherId,
  PGconn *conn
) 
{
  // get device.WARNING:  there is already a transaction in progress
  uint64_t nwatcherid = htonll(watcher.id());
  const char *values[1] = { (const char *) &nwatcherid };
  int lengths[1] = { (int) sizeof(nwatcherid) };
  int binary[1] = { 1 };
  
  std::string q = "SELECT id, \"start\", status, \"rights\", name, login, \"password\", tag FROM watcher WHERE id = $1";
  PGresult *r = PQexecParams(conn, q.c_str(), 1, NULL, values, lengths, binary, 0);

  if (PQresultStatus(r) != PGRES_TUPLES_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }
	int rows = PQntuples(r);
  if (rows <= 0)
    return ERR_CODE_GET_WATCHER;
  char *v = PQgetvalue(r, 0, 0);
  watcher.set_id(strtoull(v, NULL, 10));
  v = PQgetvalue(r, 0, 1);
  watcher.set_start(strtoull(v, NULL, 10));
  v = PQgetvalue(r, 0, 2);
  watcher.set_status(strtol(v, NULL, 10));
  v = PQgetvalue(r, 0, 3);
  watcher.set_rights(strtol(v, NULL, 10));
  v = PQgetvalue(r, 0, 4);
  watcher.set_name(v);
  v = PQgetvalue(r, 0, 5);
  watcher.set_login(v);
  v = PQgetvalue(r, 0, 6);
  watcher.set_password(v);
  v = PQgetvalue(r, 0, 7);
  watcher.set_tag(strtol(v, NULL, 10));
  PQclear(r);
  return TRACKER_OK;
}

static int loadWatchers(
  std::vector<gpstracker::Watcher> &watchers,
  uint64_t watcherId,
  int32_t rights,
  size_t maxcount,
  size_t offset,
  PGconn *conn
) 
{
  bool isAdmin = (rights & 1) != 0;
  if (!isAdmin)
    return ERR_CODE_GET_WATCHER;

  // get device.WARNING:  there is already a transaction in progress
  std::stringstream q;
  PGresult *r;
  
  q << "SELECT id, \"start\", status, \"rights\", name, login, \"password\", tag FROM watcher ORDER BY id LIMIT "
    << maxcount << " OFFSET " << offset;
  r = PQexec(conn, q.str().c_str());

  if (PQresultStatus(r) != PGRES_TUPLES_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }
	int rows = PQntuples(r);
  for (int row = 0; row < rows; row++) {
    gpstracker::Watcher watcher;
    char *v = PQgetvalue(r, row, 0);
    watcher.set_id(strtoull(v, NULL, 10));
    v = PQgetvalue(r, row, 1);
    watcher.set_start(strtoull(v, NULL, 10));
    v = PQgetvalue(r, row, 2);
    watcher.set_status(strtol(v, NULL, 10));
    v = PQgetvalue(r, row, 3);
    watcher.set_rights(strtol(v, NULL, 10));
    v = PQgetvalue(r, row, 4);
    watcher.set_name(v);
    v = PQgetvalue(r, row, 5);
    watcher.set_login(v);
    v = PQgetvalue(r, row, 6);
    watcher.set_password(v);
    v = PQgetvalue(r, row, 7);
    watcher.set_tag(strtol(v, NULL, 10));
    watchers.push_back(watcher);
  }
  PQclear(r);
  return rows > 0 ? TRACKER_OK : ERR_CODE_GET_DEVICE;
}

/**
 * @return deviceid of the track or 0 if not found
 */
static uint64_t deviceIdOfTrack(
  const uint64_t trackId,
  PGconn *conn
) 
{
  uint64_t deviceId = 0;
  uint64_t ntrackid = htonll(trackId);
  
  const char *values[1] = { (const char *) &ntrackid };
  int lengths[1] = { (int) sizeof(ntrackid) };
  int binary[1] = { 1 };

  std::string q = "SELECT deviceid FROM track where id = $1";
  PGresult *r = PQexecParams(conn, q.c_str(), 1, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_TUPLES_OK) {
    PQclear(r);
    return deviceId;
  }
  if (PQntuples(r) > 0) {
    char *v = PQgetvalue(r, 0, 0);
    deviceId = strtoull(v, NULL, 10);
  }
  PQclear(r);
  return deviceId;
}

static int loadRawData(
  PGconn *conn,
  gpstracker::RawData *rawdata,
  uint64_t trackid
)
{
  uint64_t ntrackid = htonll(trackid);

  const char *values[1] = { (const char *) &ntrackid };
  int lengths[1] = { (int) sizeof(ntrackid) };
  int binary[1] = { 1 };

  PGresult *r;
  std::string q = "SELECT id, receivedtime, srcaddr, srcport, hex, trackid, tag FROM rawdata WHERE trackid = $1 LIMIT 1";
  r = PQexecParams(conn, q.c_str(), 1, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_TUPLES_OK) {
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }
  bool rr = false;
  if (PQntuples(r) > 0 && rawdata) {
    char *v = PQgetvalue(r, 0, 0);
    rawdata->set_id(strtoull(v, NULL, 10));
    v = PQgetvalue(r, 0, 1);
    rawdata->set_receivedtime(strtoull(v, NULL, 10));
    v = PQgetvalue(r, 0, 2);
    rawdata->set_srcaddr(v);
    v = PQgetvalue(r, 0, 3);
    rawdata->set_srcport(strtoul(v, NULL, 10));
    v = PQgetvalue(r, 0, 4);
    rawdata->set_hex(v);
    v = PQgetvalue(r, 0, 5);
    rawdata->set_trackid(strtoull(v, NULL, 10));
    v = PQgetvalue(r, 0, 6);
    rawdata->set_tag(strtol(v, NULL, 10));
  }
  PQclear(r);
  return rr;
}

/**
 * @return true if watcher login/password is corrects and device of the track owned by the user(or user is admin)
 */
static bool watcherOwnTrack
(
  PGconn *conn,
  const gpstracker::Watcher &watcher,
  const gpstracker::Track &track
) 
{
  uint64_t uid;
  int rights = getWatcherRights(conn, uid, watcher);
  if (rights <= 0)
    return false;

  uint64_t deviceId = track.deviceid();
  if (deviceId == 0)
    deviceId = deviceIdOfTrack(track.id(), conn);

  bool isAdmin = (rights & 1) != 0;

  uint64_t ndeviceid = htonll(deviceId);
  uint64_t nwatcherid = htonll(watcher.id());

  const char *values[2] = { (const char *) &ndeviceid, (const char *) &nwatcherid };
  int lengths[2] = { (int) sizeof(ndeviceid), (int) sizeof(nwatcherid)  };
  int binary[2] = { 1, 1 };

  std::stringstream q;
  PGresult *r;
  if (isAdmin) {
    q << "SELECT COUNT(id) FROM device WHERE id = $1";
  } else {
    q << "SELECT COUNT(id) FROM watcher2device WHERE deviceid = $1 \
    AND watcherid = $2";
  }
  r = PQexecParams(conn, q.str().c_str(), isAdmin ? 1 : 2, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_TUPLES_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }
  bool rr = false;
  if (PQntuples(r) > 0) {
    char *v = PQgetvalue(r, 0, 0);
    rr = strtol(v, NULL, 10) > 0;
  }
  PQclear(r);
  return rr;
}

/**
 * Update device
 */
static bool updateDevice(
  PGconn *conn,
  const gpstracker::Device& device,
  const std::string& newpassword
) 
{
  uint64_t ndeviceid = htonll(device.id());
  int32_t ntag = htonl(device.tag());

  const char *values[6] = { 
    (const char *) &ndeviceid, 
    (const char *) device.name().c_str(),
    (const char *) device.color().c_str(), 
    (const char *) device.imei().c_str(),
    (const char *) newpassword.c_str(),
    (const char *) &ntag
  };
  int lengths[6] = {
    (int) sizeof(ndeviceid),
    (int) device.name().size(),
    (int) device.color().size(),
    (int) device.imei().size(),
    (int) newpassword.size(),
    (int) sizeof(ntag)
  };
  int binary[6] = { 1, 0, 0, 0, 0, 1 };

  // get device.WARNING:  there is already a transaction in progress
  std::string q = "UPDATE device SET \"name\" = $2, color = $3, imei = $4, \"password\" = $5, tag = $6 WHERE id = $1";

  PGresult *r = PQexecParams(conn, q.c_str(),
    6, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return false;
  }
  PQclear(r);
  return true;
}

/**
 * Add/update device
 */
static bool addDevice(
  PGconn *conn,
  const gpstracker::Device& device
) 
{
  uint64_t ndeviceid = htonll(device.id());
  int32_t ntag = htonl(device.tag());

  const char *values[6] = { 
    (const char *) &ndeviceid, 
    (const char *) device.name().c_str(),
    (const char *) device.color().c_str(),
    (const char *) device.imei().c_str(),
    (const char *) device.password().c_str(),
    (const char *) &ntag
  };
  int lengths[6] = {
    (int) sizeof(ndeviceid),
    (int) device.name().size(),
    (int) device.color().size(),
    (int) device.imei().size(),
    (int) device.password().size(),
    (int) sizeof(ntag)
  };
  int binary[6] = { 1, 0, 0, 0, 0, 1 };

  // get device.WARNING:  there is already a transaction in progress
  PGresult *r = PQexecParams(conn, "INSERT INTO device (id, \"name\", color, imei, \"password\", tag) VALUES (\
    $1, $2, $3, $4, $5, $6)",
    6, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return false;
  }
  PQclear(r);
  return true;
}

/**
 * Update watcher
 */
static bool updateWatcher(
  PGconn *conn,
  const gpstracker::Watcher& watcher
) 
{
  uint64_t nwatcherid = htonll(watcher.id());
  int32_t nstatus = htonl(watcher.status());
  int32_t nrights = htonl(watcher.rights());
  int32_t ntag = htonl(watcher.tag());

  const char *values[7] = { 
    (const char *) &nwatcherid, 
    (const char *) &nstatus, 
    (const char *) &nrights, 
    (const char *) watcher.name().c_str(),
    (const char *) watcher.login().c_str(),
    (const char *) watcher.password().c_str(),
    (const char *) &ntag
  };
  int lengths[7] = {
    (int) sizeof(nwatcherid),
    (int) sizeof(nstatus),
    (int) sizeof(nrights),
    (int) watcher.name().size(),
    (int) watcher.login().size(),
    (int) watcher.password().size(),
    (int) sizeof(ntag)
  };
  int binary[7] = { 1, 1, 1, 0, 0, 0, 1 };

  // get device.WARNING:  there is already a transaction in progress
  std::string q = "UPDATE watcher SET status = $2, \"rights\" = $3, \"name\" = $4, login = $5, \"password\" = $6, tag = $7 WHERE id = $1";

  PGresult *r = PQexecParams(conn, q.c_str(),
    6, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return false;
  }
  PQclear(r);
  return true;
}

/**
 * Add watcher
 */
static bool addWatcher(
  PGconn *conn,
  const gpstracker::Watcher& watcher
) 
{
  int32_t nstatus = htonl(watcher.status());
  int32_t nrights = htonl(watcher.rights());
  int32_t ntag = htonl(watcher.tag());

  const char *values[6] = { 
    (const char *) &nstatus, 
    (const char *) &nrights, 
    (const char *) watcher.name().c_str(),
    (const char *) watcher.login().c_str(),
    (const char *) watcher.password().c_str(),
    (const char *) &ntag
  };
  int lengths[6] = {
    (int) sizeof(nstatus),
    (int) sizeof(nrights),
    (int) watcher.name().size(),
    (int) watcher.login().size(),
    (int) watcher.password().size(),
    (int) sizeof(ntag)
  };
  int binary[6] = { 1, 1, 0, 0, 0, 1 };

  // get device.WARNING:  there is already a transaction in progress
  PGresult *r = PQexecParams(conn, "INSERT INTO watcher (\"status\", \"rights\", \"name\", login, \"password\", tag) VALUES (\
    $1, $2, $3, $4, $5, $6)",
    6, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return false;
  }
  PQclear(r);
  return true;
}

static bool addTrack(
  PGconn *conn,
  const gpstracker::Track& track
) {
  uint64_t deviceid = htonll(track.id());
  uint32_t gpstime = htonl(track.gpstime());
  std::string lat = double2string(track.lat(), 10);
  std::string lon = double2string(track.lon(), 10);
  std::string speed = double2string(track.speed(), 6);
  std::string direction = double2string(track.direction(), 6);

  const char *values[6] = { 
    (const char *) &deviceid,
    (const char *) &gpstime,
    (const char *) lat.c_str(),
    (const char *) lon.c_str(),
    (const char *) speed.c_str(),
    (const char *) direction.c_str()
  };
  int lengths[6] = { (int) sizeof(deviceid), (int) sizeof(gpstime),
    (int) lat.size(), (int) lon.size(),
    (int) speed.size(), (int) direction.size()
  };
  int binary[6] = { 1, 1, 0, 0, 0, 0 };

  PGresult *r = PQexecParams(conn, "INSERT INTO track \
      (deviceid, gpstime, lat, lon, speed, direction) VALUES \
      ($1, $2, $3, $4, $5, $6) RETURNING id",
    6, NULL, values, lengths, binary, 0);
  
  bool rr = PQresultStatus(r) == PGRES_COMMAND_OK;
  PQclear(r);
  return rr;
}

/**
 * Remove device
 */
static bool rmDevice(
  PGconn *conn,
  const gpstracker::Device& device
) 
{
  // std::cerr << "rmDevice " << device.id() << std::endl;

  uint64_t ndeviceid = htonll(device.id());

  const char *values[1] = { (const char *) &ndeviceid };
  int lengths[1] = { (int) sizeof(ndeviceid) };
  int binary[1] = { 1 };

  PGresult *r = PQexecParams(conn, "DELETE  FROM device \
    WHERE id = $1",
    1, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return false;
  }
  PQclear(r);
  return true;
}

/**
 * Remove watcher
 */
static bool rmWatcher(
  PGconn *conn,
  const gpstracker::Watcher& watcher
) 
{
  // std::cerr << "rmWatcher " << watcher.id() << std::endl;

  uint64_t nwatcherid = htonll(watcher.id());

  const char *values[1] = { (const char *) &nwatcherid };
  int lengths[1] = { (int) sizeof(nwatcherid) };
  int binary[1] = { 1 };

  PGresult *r = PQexecParams(conn, "DELETE  FROM watcher WHERE id = $1",
    1, NULL, values, lengths, binary, 0);
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return false;
  }
  PQclear(r);
  return true;
}

/**
 * Load tracks into device in time window
 */
static int loadTracks(
  gpstracker::Device& device,
  PGconn *conn,
  uint64_t deviceid,
  uint32_t start,
  uint32_t finish,
  uint32_t count
) 
{
  uint64_t ndeviceid = htonll(deviceid);
  uint32_t nstart = htonl(start);
  uint32_t nfinish = htonl(finish);
  uint32_t ncount = htonl(count);

  // std::cerr << "loadTracks " << deviceid << " " << start << " " << finish << " " << finish << " " << count << std::endl;

  const char *values[4] = { (const char *) &ndeviceid, (const char *) &nstart, (const char *) &nfinish, (const char *) &ncount };
  int lengths[4] = { (int) sizeof(ndeviceid), (int) sizeof(nstart), (int) sizeof(nfinish), (int) sizeof(ncount) };
  int binary[4] = { 1, 1, 1, 1 };

  // get track in time window if exists.
  std::stringstream ss;
  ss << "SELECT id, gpstime, lat, lon, speed, direction, tag FROM track \
    WHERE deviceid = $1 AND gpstime >= $2 AND gpstime <= $3 \
    ORDER BY gpstime DESC LIMIT " << count; 
  PGresult *r = PQexecParams(conn, ss.str().c_str(),
    3, NULL, values, lengths, binary, 0); // 3, $4 cause ERROR: insufficient data left in message
  if (PQresultStatus(r) != PGRES_TUPLES_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return ERR_CODE_GET_TRACK;
  }
	int rows = PQntuples(r);
  // set tracks
	for (int row = 0; row < rows; row++) {
    gpstracker::Track *item = device.add_track();
    char *v = PQgetvalue(r, row, 0);
    item->set_id(strtoull(v, NULL, 10));
    v = PQgetvalue(r, row, 1);
    item->set_gpstime(strtoul(v, NULL, 10));
    v = PQgetvalue(r, row, 2);
    item->set_lat(strtod(v, NULL));
    v = PQgetvalue(r, row, 3);
    item->set_lon(strtod(v, NULL));
    v = PQgetvalue(r, row, 4);
    item->set_speed(strtod(v, NULL));
    v = PQgetvalue(r, row, 5);
    item->set_direction(strtod(v, NULL));
    v = PQgetvalue(r, row, 6);
    item->set_tag(strtol(v, NULL, 10));
  }
  PQclear(r);
  return TRACKER_OK;
}

/**
 * Load count tracks into device earlier finish
 */
static int loadCountTracks(
  gpstracker::Device& device,
  PGconn *conn,
  uint64_t deviceid,
  uint32_t before,
  uint32_t count
) 
{
  uint64_t ndeviceid = htonll(deviceid);
  uint32_t nbefore = htonl(before);
  uint32_t ncount = htonl(count);

  const char *values[3] = { (const char *) &ndeviceid, (const char *) &nbefore, (const char *) &ncount  };
  int lengths[3] = { (int) sizeof(ndeviceid), (int) sizeof(nbefore), (int) sizeof(ncount) };
  int binary[3] = { 1, 1, 1 };

  // get tracks
  std::stringstream ss;
  ss << "SELECT id, gpstime, lat, lon, speed, direction, tag FROM track \
    WHERE deviceid = $1 AND gpstime <= $2 \
    ORDER BY gpstime DESC LIMIT " << count;
  PGresult *r = PQexecParams(conn, ss.str().c_str(),
    2, NULL, values, lengths, binary, 0);  // 3, $3 cause ERROR: insufficient data left in message
  if (PQresultStatus(r) != PGRES_TUPLES_OK) {
    std::cerr << PQerrorMessage(conn) << std::endl;
    PQclear(r);
    return ERR_CODE_GET_TRACK;
  }
	int rows = PQntuples(r);
  // set tracks
	for (int row = 0; row < rows; row++) {
    gpstracker::Track *item = device.add_track();
    char *v = PQgetvalue(r, row, 0);
    item->set_id(strtoull(v, NULL, 10));
    v = PQgetvalue(r, row, 1);
    item->set_gpstime(strtoul(v, NULL, 10));
    v = PQgetvalue(r, row, 2);
    item->set_lat(strtod(v, NULL));
    v = PQgetvalue(r, row, 3);
    item->set_lon(strtod(v, NULL));
    v = PQgetvalue(r, row, 4);
    item->set_speed(strtod(v, NULL));
    v = PQgetvalue(r, row, 5);
    item->set_direction(strtod(v, NULL));
    v = PQgetvalue(r, row, 6);
    item->set_tag(strtol(v, NULL, 10));
  }
  PQclear(r);
  return TRACKER_OK;
}

/**
 * @see https://habr.com/ru/post/340758/
 * @see https://github.com/Mityuha/grpc_async/blob/master/grpc_async_server.cc
 */
GpsTrackMgrServiceImpl::GpsTrackMgrServiceImpl(
  const std::string &aconninfo,
  const std::string &agrpc_listener_address,
  bool allow_user_add_device
)
  : address(agrpc_listener_address), conninfo(aconninfo), allowUserAddDevice(allow_user_add_device),
  clientsListenPg(this), queuingMgr(NULL)
{
  ConnStatusType status = start();
  if (status == CONNECTION_BAD) {
      std::cerr << ERR_PG_CONNECTION << PQerrorMessage(conn) << std::endl;
  }
  if (status == CONNECTION_STARTED) {
      std::cerr << MSG_PG_CONNECTING << std::endl;
  }
}

void GpsTrackMgrServiceImpl::stop() {
  running = false;
  // stop service
  gpr_timespec timeout {10, 0};

  server->Shutdown(timeout);
  // Always shutdown the completion queue AFTER the server.
  cq->Shutdown();
  
  if (queuingMgr) {
    delete queuingMgr;
    queuingMgr = NULL;
  }
  
  // stop listen Postgres
  // clientsListenPg.stop();
  // stop postgres client connection
  if (conn) {
    PQfinish(conn);
    conn = NULL;
  }
}

ConnStatusType GpsTrackMgrServiceImpl::start() {
  // Postgres

  conn = PQconnectdb(conninfo.c_str());
  ConnStatusType status = PQstatus(conn);

  // GRPC listener
  grpc::ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());

  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *asynchronous* service.
  builder.RegisterService(&service); 

  // Get hold of the completion queue used for the asynchronous communication
  // with the gRPC runtime.
  cq = builder.AddCompletionQueue();

  // after cq created
  queuingMgr = new QueuingMgr(this);

  // Finally assemble the server.
  server = builder.BuildAndStart();

  // start database listener  
  clientsListenPg.start(conninfo);

  return status;
}

bool GpsTrackMgrServiceImpl::checkConnStatus() {
  bool r = true; 
  if (PQstatus(conn) == CONNECTION_BAD) {
    conn = PQconnectdb(conninfo.c_str());
    r = (PQstatus(conn) == CONNECTION_OK);
    r = (PQstatus(conn) == CONNECTION_OK);
    if (r) {
      clientsListenPg.start(conninfo);
    }
  }
  return r;
}

void GpsTrackMgrServiceImpl::run()
{
  // Spawn a new instances to serve new clients.
  new DeviceTrackData(this);
  new LsDevicesData(this);
  new LsWatchersData(this);
  new ChDeviceData(this);
  new ChWatcherData(this);
  new AddDevicesData(this);
  new ListenTrackData(this);

  void* tag;  // uniquely identifies a request.
  bool successfulEvent;
  running = true;
  checkConnStatus();
  while (running) {
    // Block waiting to read the next event from the completion queue. The
    // event is uniquely identified by its tag, which in this case is the
    // memory address of an instance.
    // The return value of Next should always be checked. This return value
    // tells us whether there is any kind of event or cq_ is shutting down.
    if (!cq->Next(&tag, &successfulEvent))
      break;
    static_cast<CommonCallData*>(tag)->Proceed(successfulEvent);
  }
}

GpsTrackMgrServiceImpl::~GpsTrackMgrServiceImpl()
{
  stop();
}

// Take in the "service" instance (in this case representing an asynchronous
// server) and the completion queue "cq" used for asynchronous communication
// with the gRPC runtime.
CommonCallData::CommonCallData(
  gpstracker::monitor::AsyncService* aservice,
  grpc::ServerCompletionQueue* acq) 
  : service(aservice), cq(acq), status(CREATE)
{
}

/**
 * =========== DeviceTrackData ===========
 */
DeviceTrackData::DeviceTrackData(
  GpsTrackMgrServiceImpl *service
) : CommonCallData(&service->service, service->cq.get()), 
  responder(&ctx), service(service), new_responder_created(false)
{
  Proceed();
}

int DeviceTrackData::load(
  bool isAdmin
) {
  service->checkConnStatus();
  PGresult *r = PQexec(service->conn, "BEGIN");
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    // std::cerr << PQerrorMessage(service->conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }
  result.set_id(request.device().id());
  result.set_password(request.device().password());
  
  if (loadDevice(isAdmin, result, service->conn) != 0) {
    rollback(service->conn, r);
    return ERR_CODE_PG_RESULT;
  }
  size_t limit = request.timewindow().limit();
  if (limit <= 0) {
    limit = DEF_TRACKS_COUNT;
  }
  if (loadTracks(result, service->conn, result.id(), request.timewindow().start(), request.timewindow().finish(), limit) != 0) {
    rollback(service->conn, r);
    PQclear(r);
    return ERR_CODE_PG_RESULT;
	}
  if (result.track_size() == 0) {
    loadCountTracks(result, service->conn, result.id(), request.timewindow().finish(), limit);
  }
  r = PQexec(service->conn, "COMMIT");
  PQclear(r);
  return 0;
}

void DeviceTrackData::Proceed(bool successfulEvent) {
  switch (status) {
  case CREATE:
    status = PROCESS;
    service->service.RequestdeviceTrack(&ctx, &request, &responder, cq, cq, this);
    break;
  case PROCESS:
    if (!new_responder_created) {
      new DeviceTrackData(service);
      new_responder_created = true;
      load(false);
      responder.Finish(result, grpc::Status(), (void*) this);
    }
    status = FINISH;
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== LsDevicesData ===========
 */

LsDevicesData::LsDevicesData(
  GpsTrackMgrServiceImpl *service
) : CommonCallData(&service->service, service->cq.get()), 
  service(service), new_responder_created(false), counter(0), responder(&ctx)
{
  Proceed();
}

int LsDevicesData::load() {
  service->checkConnStatus();
  PGresult *r = PQexec(service->conn, "BEGIN");
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    // std::cerr << PQerrorMessage(service->conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }

  uint64_t watcherId;
  int32_t rights = getWatcherRights(service->conn, watcherId, request.watcher());

  size_t sz = request.timewindow().limit();
  if ((sz <= 0) || (sz > DEF_DEVICE_COUNT))
    sz = DEF_DEVICE_COUNT;

  size_t ofs = request.timewindow().start();

  if (loadDevices(result, watcherId, rights, sz, ofs, service->conn) != 0) {
    rollback(service->conn, r);
    return ERR_CODE_PG_RESULT;
  }

  r = PQexec(service->conn, "COMMIT");
  PQclear(r);
  return 0;
}

void LsDevicesData::Proceed(bool successfulEvent) {
  switch (status) {
  case CREATE:
    status = PROCESS;
    service->service.RequestlsDevices(&ctx, &request, &responder, cq, cq, this);
    break;
  case PROCESS:
    if (!new_responder_created) {
      new LsDevicesData(service);
      new_responder_created = true;
      load();
    }
    if (!successfulEvent || counter >= result.size()) {
      status = FINISH;
      responder.Finish(grpc::Status(), (void*) this);
    } else {
      responder.Write(result[counter], (void*) this);
      counter++;
    }
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== LsWatchersData ===========
 */

LsWatchersData::LsWatchersData(
  GpsTrackMgrServiceImpl *service
) : CommonCallData(&service->service, service->cq.get()), 
  service(service), new_responder_created(false), counter(0), responder(&ctx)
{
  Proceed();
}

int LsWatchersData::load() {
  service->checkConnStatus();
  PGresult *r = PQexec(service->conn, "BEGIN");
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    // std::cerr << PQerrorMessage(service->conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }

  uint64_t watcherId;
  int32_t rights = getWatcherRights(service->conn, watcherId, request.watcher());

  size_t sz = request.timewindow().limit();
  if ((sz <= 0) || (sz > DEF_DEVICE_COUNT))
    sz = DEF_DEVICE_COUNT;

  size_t ofs = request.timewindow().start();

  if (loadWatchers(result, watcherId, rights, sz, ofs, service->conn) != 0) {
    rollback(service->conn, r);
    return ERR_CODE_PG_RESULT;
  }

  r = PQexec(service->conn, "COMMIT");
  PQclear(r);
  return 0;
}

void LsWatchersData::Proceed(bool successfulEvent) {
  switch (status) {
  case CREATE:
    status = PROCESS;
    service->service.RequestlsWatchers(&ctx, &request, &responder, cq, cq, this);
    break;
  case PROCESS:
    if (!new_responder_created) {
      new LsWatchersData(service);
      new_responder_created = true;
      load();
    }
    if (!successfulEvent || counter >= result.size()) {
      status = FINISH;
      responder.Finish(grpc::Status(), (void*) this);
    } else {
      responder.Write(result[counter], (void*) this);
      counter++;
    }
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== ChDeviceData ===========
 */
ChDeviceData::ChDeviceData(
  GpsTrackMgrServiceImpl *service
) : CommonCallData(&service->service, service->cq.get()), 
  responder(&ctx), service(service), new_responder_created(false)
{
  Proceed();
}

int ChDeviceData::load() {
  service->checkConnStatus();
  PGresult *r = PQexec(service->conn, "BEGIN");
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    // std::cerr << PQerrorMessage(service->conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }
  result.set_id(request.device().id());
  result.set_password(request.device().password());

  bool isAdmin = hasAdminRights(service->conn, request.watcher());

  if (request.op() != 2) {  // not to add a new device
    if (loadDevice(isAdmin, result, service->conn) != 0) {
      std::cerr << "Error load device id: " << result.id() << ", password: " << result.password() << std::endl;
      rollback(service->conn, r);
      return ERR_CODE_GET_DEVICE;
    }
  }
  
  // 0- get, 1- change, 2- add(clear), 3- rm  
  switch (request.op()) {
    case 1: // change
      if (!updateDevice(service->conn, request.device(), 
        request.newdevicepassword().empty() ? request.device().password() : request.newdevicepassword())) {
        rollback(service->conn, r);
        return ERR_CODE_SET_DEVICE;
        }
      break;
    case 2: // add(clear)
      if ((!service->allowUserAddDevice) && (!isAdmin)) {
        rollback(service->conn, r);
        return ERR_CODE_UNAUTHORIZED;
      }
      if (!addDevice(service->conn, request.device())) {
        rollback(service->conn, r);
        return ERR_CODE_ADD_DEVICE;
      }
      break;
    case 3: // rm
      if (!isAdmin) {
        rollback(service->conn, r);
        return ERR_CODE_UNAUTHORIZED;
      }
      if (!rmDevice(service->conn, request.device())) {
        rollback(service->conn, r);
        return ERR_CODE_RM_DEVICE;
      } 
      break;
    default:
      break;
  }
  r = PQexec(service->conn, "COMMIT");
  PQclear(r);
  return TRACKER_OK;
}

void ChDeviceData::Proceed(bool successfulEvent) {
  switch (status) {
  case CREATE:
    status = PROCESS;
    service->service.RequestchDevice(&ctx, &request, &responder, cq, cq, this);
    break;
  case PROCESS:
    if (!new_responder_created) {
      new ChDeviceData(service);
      new_responder_created = true;
      
      grpc::StatusCode c;
      {
        int code = load();
        std::stringstream ss;
        if (code != 0) {
          ss << "Error " << code << ": " << strerror_gps(code);
          c = grpc::UNKNOWN;
        } else {
          c = grpc::OK;
        }
        responder.Finish(result, grpc::Status(c, ss.str()), (void*) this);
      }
    }
    status = FINISH;
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== ChWatcherData ===========
 */
ChWatcherData::ChWatcherData(
  GpsTrackMgrServiceImpl *service
) : CommonCallData(&service->service, service->cq.get()), 
  responder(&ctx), service(service), new_responder_created(false)
{
  Proceed();
}

int ChWatcherData::load() {
  service->checkConnStatus();
  PGresult *r = PQexec(service->conn, "BEGIN");
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    // std::cerr << PQerrorMessage(service->conn) << std::endl;
    PQclear(r);
    return ERR_CODE_PG_RESULT;
  }
  bool isAdmin = hasAdminRights(service->conn, request.watcher());
  if (!isAdmin)
    return ERR_CODE_GET_WATCHER;

  result.set_id(request.user().id());
  result.set_password(request.user().password());

  if (request.op() != 2) {  // not to add a new device
    if (loadWatcher(result, request.user().id(), service->conn) != 0) {
      std::cerr << "Error load watcher id: " << result.id() << ", password: " << result.password() << std::endl;
      rollback(service->conn, r);
      return ERR_CODE_GET_WATCHER;
    }
  }
  
  // 0- get, 1- change, 2- add(clear), 3- rm  
  switch (request.op()) {
    case 1: // change
      if (!updateWatcher(service->conn, request.user())) {
        rollback(service->conn, r);
        return ERR_CODE_SET_WATCHER;
        }
      break;
    case 2: // add(clear)
      if (!addWatcher(service->conn, request.user())) {
        rollback(service->conn, r);
        return ERR_CODE_ADD_WATCHER;
      }
      break;
    case 3: // rm
      if (!rmWatcher(service->conn, request.user())) {
        rollback(service->conn, r);
        return ERR_CODE_RM_WATCHER;
      } 
      break;
    default:
      break;
  }
  r = PQexec(service->conn, "COMMIT");
  PQclear(r);
  return TRACKER_OK;
}

void ChWatcherData::Proceed(bool successfulEvent) {
  switch (status) {
  case CREATE:
    status = PROCESS;
    service->service.RequestchWatcher(&ctx, &request, &responder, cq, cq, this);
    break;
  case PROCESS:
    if (!new_responder_created) {
      new ChWatcherData(service);
      new_responder_created = true;
      
      grpc::StatusCode c;
      {
        int code = load();
        std::stringstream ss;
        if (code != 0) {
          ss << "Error " << code << ": " << strerror_gps(code);
          c = grpc::UNKNOWN;
        } else {
          c = grpc::OK;
        }
        responder.Finish(result, grpc::Status(c, ss.str()), (void*) this);
      }
    }
    status = FINISH;
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== AddDevicesData ===========
 */
AddDevicesData::AddDevicesData(
  GpsTrackMgrServiceImpl *service
) : CommonCallData(&service->service, service->cq.get()), 
  responder(&ctx), service(service), new_responder_created(false)
{
  Proceed();
}

int AddDevicesData::put() {
  service->checkConnStatus();
  if (!addDevice(service->conn, request)) {
    return ERR_CODE_ADD_DEVICE;
  }
  return TRACKER_OK;
}

void AddDevicesData::Proceed(bool successfulEvent) {
  switch (status) {
  case CREATE:
    status = PROCESS;
    service->service.RequestaddDevices(&ctx, &responder, cq, cq, this);
    break;
  case PROCESS:
    if (!new_responder_created) {
      new AddDevicesData(service);
      new_responder_created = true;
    }
      
    if (!successfulEvent) {
      status = FINISH;
      responder.Finish(result, grpc::Status(), (void*) this);
      return;
    }
    responder.Read(&request, (void*)this);
    {
      // first read incomplete
      uint64_t id = request.id();
      if (id) {
        int code = put();
        if (code == 0) {
          result.set_count(result.count() + 1);
        }
      }
    }
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== AddTracksData ===========
 */
AddTracksData::AddTracksData(
  GpsTrackMgrServiceImpl *service
) : CommonCallData(&service->service, service->cq.get()), 
  responder(&ctx), service(service), new_responder_created(false)
{
  Proceed();
}

int AddTracksData::put() {
  service->checkConnStatus();
  if (!addTrack(service->conn, request)) {
    return ERR_CODE_ADD_TRACK;
  }
  return TRACKER_OK;
}

void AddTracksData::Proceed(bool successfulEvent) {
  switch (status) {
  case CREATE:
    status = PROCESS;
    service->service.RequestaddTracks(&ctx, &responder, cq, cq, this);
    break;
  case PROCESS:
    if (!new_responder_created) {
      new AddTracksData(service);
      new_responder_created = true;
    }
      
    if (!successfulEvent) {
      status = FINISH;
      responder.Finish(result, grpc::Status(), (void*) this);
      return;
    }
    responder.Read(&request, (void*)this);
    {
      int code = put();
      if (code == 0)  {
        result.set_count(result.count() + 1);
      }
    }
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== ListenTrackData ===========
 */
ListenTrackData::ListenTrackData(
  GpsTrackMgrServiceImpl *track_service
) : trackService(track_service), CommonCallData(&track_service->service, track_service->cq.get()), 
    responder(&ctx), new_responder_created(false)
{
  ctx.AsyncNotifyWhenDone(this);
  Proceed();
}

/**
 * @see https://www.gresearch.co.uk/2019/03/20/lessons-learnt-from-writing-asynchronous-streaming-grpc-services-in-c/
 */
void ListenTrackData::enqueue(
  const std::vector<gpstracker::Track> &values
) {
  // completion queue alarm
  for (std::vector<gpstracker::Track>::const_iterator it = values.begin(); it != values.end(); it++) {
    if (isWatcherCanSeeTrack(rq, *it, NULL))
      result.push(*it);
  }
  if (result.empty())
    return;
  if (status == LISTEN_TRACK) {
    grpc::Alarm alarm;
    alarm.Set(cq, gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME), this);
  }
}

bool ListenTrackData::isWatcherCanSeeTrack
(
  const gpstracker::Watcher &watcher,
  const gpstracker::Track &track,
  gpstracker::RawData *rawdata
) {
  trackService->checkConnStatus();
  PGresult *r = PQexec(trackService->conn, "BEGIN");
  if (PQresultStatus(r) != PGRES_COMMAND_OK) {
    PQclear(r);
    return false;
  }
  bool rr = watcherOwnTrack(trackService->conn, watcher, track);
  if (rawdata)
    loadRawData(trackService->conn, rawdata, track.id());
  r = PQexec(trackService->conn, "COMMIT");
  PQclear(r);
  return rr;
}

bool ListenTrackData::writeNext()
{
  if (result.empty())
    return false;
  gpstracker::RawData rd;
  gpstracker::Track trk = result.front();
  result.pop();
  gpstracker::TrackNRawData tr;
  if (isWatcherCanSeeTrack(rq, trk, &rd)) {
    *tr.mutable_track() = trk;
    *tr.mutable_rawdata() = rd;
  }
  responder.Write(tr, (void*) this);
  return true;
}

/**
 *
 * Before call ctx.IsCancelled() you must call ctx.AsyncNotifyWhenDone(this), in constructor
 * After ctx.AsyncNotifyWhenDone(this), GRPC generate event when client has been occasionally disconnected and "send event" to the  Proceed() 
 * You must check is user disconnected using ctx.IsCancelled() in the Proceed() 
 * 
 * Another approach is do not call ctx.AsyncNotifyWhenDone(this), in constructor.
 * In cis case ctx.IsCancelled() cause AVE.
 * You dont need check is user disconnected, and there no notice except 
 * check paramater successfulEvent = false 
 *  
 */
void ListenTrackData::Proceed(bool successfulEvent) 
{
  switch (status) {
  case CREATE:
    status = LISTEN_TRACK;
    service->RequestlistenTrack(&ctx, &rq, &responder, cq, cq, this);
    break;
  case LISTEN_TRACK:
    if (!new_responder_created) {
      new ListenTrackData(trackService);
      new_responder_created = true;
      trackService->clientsListenPg.put(this);
      break;
		}
    if (result.empty()) { // if (ctx.IsCancelled()) { == ctx.AsyncNotifyWhenDone(this);
      status = FINISH;
      trackService->clientsListenPg.rm(this);
      responder.Finish(grpc::Status(), (void*) this);
      break;
    }
    writeNext();
    status = LISTEN_TRACK_SENT;
    break;
  case LISTEN_TRACK_SENT:
    if (!writeNext())
      status = LISTEN_TRACK;
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== QueuingMgr ===========
 * 
  GRPC distribution contains an example of an asynchronous service: the service sends messages to the client one by one using 
  the responder.Write() method from predefined array.
  When all elements of the array are sent, the service calls the responder's Finish().
  After that, the responder’s object is destroyed.
  In the example, the responder is a member of the object that lives until the RPC works out to the end.

  As events occur on the database server, namely, inserting new preorder records into the table,
  service in the ListenOrderData() RPC method must send messages to connected clients.
  These events can happen at any time.
  Example does not show how to do it.

  ListenPg::listenNotifications() is waiting for the database event to occur in a separate thread.
  When this happens, it calls ListenPg::handlePgRead() to read PostgreSQL NOTIFY payload and then
  calls ListenPg::notifyClients().

  ListenPg::notifyClients() puts QueuingMgr object into queue using QueuingMgr::enqueue() method.

  When the queue reaches the QueuingMgr object, QueuingMgr queues the objects from the
  listenResponders vector using enqueue().
  You cannot directly queue these objects from another thread due to a race.
 */
QueuingMgr::QueuingMgr(
  GpsTrackMgrServiceImpl *track_service
)  : trackService(track_service), CommonCallData(&track_service->service, track_service->cq.get())
{
}
    
/**
 * Push into queue itself.
 * when the queue reaches this QueuingMgr object, the Proceed() method will insert
 * objects from the vector of ListenTrackData into the queue.
 * It is important to insert ListenTrackData objects from the queue handler
 * to avoid race conditions
 * 
 * @see https://www.gresearch.co.uk/2019/03/20/lessons-learnt-from-writing-asynchronous-streaming-grpc-services-in-c/
 */
void QueuingMgr::enqueue(
  const std::vector<gpstracker::Track> &values
) {
  // completion queue alarm
  result = values;
  grpc::Alarm alarm;
  alarm.Set(cq, gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME), this);
}

/**
 * Push into queue ListenTrackData objects from vector of active clients to send notification about tracks
 */
void QueuingMgr::Proceed(bool successfulEvent) {
  // std::cerr << "QueuingMgr enqueue " << this << std::endl;
  if (result.empty())
    return;
  trackService->clientsListenPg.enqueue(result);
}
