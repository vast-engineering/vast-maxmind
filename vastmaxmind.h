// #ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#ifndef VASTMAXMIND_H
#define VASTMAXMIND_H
#include <node.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

class VastMaxmind : public node::ObjectWrap {
  public:
    char *db;
    GeoIP *gi;
    static void Init(v8::Handle<v8::Object> target);

  private:
    VastMaxmind(char *_db);
    ~VastMaxmind();

    static int translateIp(char *ipaddr);
    static void locationWorker(uv_work_t *req);
    static void locationAfter(uv_work_t *req, int status);

    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> getLocation(const v8::Arguments& args);
};

#endif
// #endif