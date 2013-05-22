#include <node.h>
#include <node_buffer.h>
#include <string.h>
#include "vastmaxmind.h"

using namespace v8;

struct iprequest {
  v8::Persistent<v8::Function> cb;
  char *addr;
  uint ipnum;
  VastMaxmind *vmm;
  GeoIPRecord *gir;
};

int VastMaxmind::translateIp(char *ipaddr) {
  char *str = new char[16];
  char *tok;
  int *parts = new int[4];
  int ipnum = 0;

  strncpy(str, ipaddr, 15);
  tok = strtok(str, "."); // should be 4 groups of digits

  for(int i=0; i < 4; i++) {
      if (tok != NULL) {
          parts[i] = atoi(tok);
          tok = strtok(NULL, ".");
      }
  }

  ipnum = 16777216*parts[0] + 65536*parts[1] + 256*parts[2] + parts[3];
  delete [] parts;  
  delete [] str;

  return ipnum;
}

void VastMaxmind::locationWorker(uv_work_t *req) {
    iprequest *ipreq = (iprequest *)(req->data);
    VastMaxmind *vmm = ipreq->vmm;


    ipreq->ipnum = VastMaxmind::translateIp(ipreq->addr);

    if (ipreq->ipnum > 0 && vmm && vmm->gi) {
        ipreq->gir = GeoIP_record_by_ipnum(vmm->gi, ipreq->ipnum);
    }
    else {
        ThrowException(Exception::Error(String::New("Maxmind db not opened.")));
    }
    
}

void VastMaxmind::locationAfter(uv_work_t *req, int status) {
    HandleScope scope;
    Handle<Object> ret = Object::New();
    iprequest *ipreq = (iprequest *)(req->data);
    char *na = (char *)"N/A";
    GeoIPRecord *gir = ipreq->gir;

    // return the searched ip
    ret->Set( NODE_PSYMBOL("ip"), String::New(ipreq->addr) );
    ret->Set( NODE_PSYMBOL("ipnum"), Number::New(ipreq->ipnum) );

    if (gir) {
        ret->Set( NODE_PSYMBOL("country"), String::New(gir->country_code ? gir->country_code : na) );
        ret->Set( NODE_PSYMBOL("state"), String::New(gir->region ? gir->region : na) );
        ret->Set( NODE_PSYMBOL("city"), String::New(gir->city ? gir->city : na) );
        ret->Set( NODE_PSYMBOL("zip"), String::New(gir->postal_code ? gir->postal_code : na) );
        ret->Set( NODE_PSYMBOL("latitude"), Number::New( gir->latitude) );
        ret->Set( NODE_PSYMBOL("longitude"), Number::New( gir->longitude) );
        ret->Set( NODE_PSYMBOL("areacode"), Number::New( gir->area_code) );
        GeoIPRecord_delete(gir);
    }

    Local<Value> argv[1];
    argv[0] = Local<Value>::Local(ret->ToObject());

    TryCatch try_catch;

    ipreq->cb->Call(Context::GetCurrent()->Global(), 1, argv);

    if (try_catch.HasCaught())
        node::FatalException(try_catch);

    ipreq->cb.Dispose();
    delete ipreq;
    delete req;
}


// Constuctor
VastMaxmind::VastMaxmind(char *_db) : db(_db) {
  this->gi = GeoIP_open(this->db, GEOIP_MEMORY_CACHE); //GEOIP_MEMORY_CACHE | GEOIP_INDEX_CACHE);
  if (this->gi == NULL) {
    ThrowException(Exception::Error(String::New("Could not open maxmind db.")));
  }
}

// Destructor
VastMaxmind::~VastMaxmind() {
  GeoIP_delete(this->gi);
  GeoIP_cleanup();
}

void VastMaxmind::Init(Handle<Object> target) {

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("VastMaxmind"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("location"),
      FunctionTemplate::New(getLocation)->GetFunction());

  v8::Persistent<Function> constructor = v8::Persistent<Function>::New(tpl->GetFunction());
  target->Set(String::NewSymbol("VastMaxmind"), constructor);
}

Handle<Value> VastMaxmind::New(const Arguments& args) {

  HandleScope scope;
  String::Utf8Value db(args[0]->ToString());

  VastMaxmind *vmm = new VastMaxmind(*db);
  vmm->Wrap(args.This());
  return args.This();
}

Handle<Value> VastMaxmind::getLocation(const Arguments& args) {
  HandleScope scope;
  String::Utf8Value ipaddr(args[0]);
  Local<Function> cb = Local<Function>::Cast(args[1]);
  VastMaxmind *vmm = ObjectWrap::Unwrap<VastMaxmind>(args.Holder());
  iprequest *request = new iprequest;

  request->cb = Persistent<Function>::New(cb);
  request->vmm = vmm;

  char *strNew = new char[ strlen(*ipaddr) + 1 ];
  strcpy(strNew, *ipaddr);
  request->addr = strNew;

  //eio_custom(EIO_location, EIO_PRI_DEFAULT, EIO_locationAfter, req);
  uv_work_t* req = new uv_work_t();
  req->data = request;
  uv_queue_work(uv_default_loop(), req, VastMaxmind::locationWorker, (uv_after_work_cb)VastMaxmind::locationAfter);

  scope.Close(Undefined());

  return Undefined(); //Handle<String>(args[0]->ToString());
}
