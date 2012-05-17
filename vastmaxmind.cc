#include <node.h>
#include <node_buffer.h>
#include <string.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

using namespace v8;
using namespace node;

struct location_result {
    char* zip;
    char* city;
    char* state;
    double latitude;
    double longitude;
    char* country;
    int areacode;
};

class VastMaxmind : public ObjectWrap {
protected:
    static Persistent<FunctionTemplate> constructor_template;
    char *db;
    GeoIP *gi;

public:
    VastMaxmind(char *_db) : db(_db) {
        this->gi = GeoIP_open(this->db, GEOIP_STANDARD);// GEOIP_MEMORY_CACHE | GEOIP_INDEX_CACHE);
        if (this->gi == NULL) {
            ThrowException(Exception::Error(String::New("Could not open maxmind db.")));
        }

    }
    ~VastMaxmind() {
        GeoIP_delete(this->gi);
    }

    static void Initialize(Handle<Object> target) {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);
        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
        constructor_template->SetClassName(String::NewSymbol("VastMaxmind"));

        NODE_SET_PROTOTYPE_METHOD(constructor_template, "location", location);
        target->Set(String::NewSymbol("VastMaxmind"), constructor_template->GetFunction());
    }

    struct iprequest {
        Persistent<Function> cb;
        char *addr;
        VastMaxmind *vmm;
        struct location_result result;
    };

    static Handle<Value> New(const Arguments &args) {
        HandleScope scope;
        String::Utf8Value db(args[0]->ToString());

        VastMaxmind *vmm = new VastMaxmind(*db);
        vmm->Wrap(args.This());
        return args.This();
    }


    static void locationWorker(uv_work_t *req) {
        iprequest *ipreq = (iprequest *)req->data;
        VastMaxmind *vmm = ipreq->vmm;
        GeoIPRecord *gir;
        char *na = (char *)"N/A";

        if (!vmm || !vmm->gi) {
            ThrowException(Exception::Error(String::New("Maxmind db not opened.")));
        }
        else {
            gir = GeoIP_record_by_ipnum(vmm->gi, inet_addr(ipreq->addr));
            // ipreq->result.country = ipreq->addr;
            // ipreq->result.areacode = inet_addr(ipreq->addr);

            ipreq->result.country = gir->country_code ? gir->country_code : na;
            ipreq->result.state = gir->region ? gir->region : na;
            ipreq->result.city = gir->city ? gir->city : na;
            ipreq->result.zip = gir->postal_code ? gir->postal_code : na;
            ipreq->result.latitude = gir->latitude;
            ipreq->result.longitude = gir->longitude;
            ipreq->result.areacode = gir->area_code;

            //GeoIPRecord_delete(gir);
        }
        
    }

    static void locationAfter(uv_work_t *req) {
        ev_unref(EV_DEFAULT_UC);

        HandleScope scope;
        Handle<Object> ret = Object::New();
        iprequest *ipreq = (iprequest *)req->data;

        ret->Set( NODE_PSYMBOL("country"), String::New(ipreq->result.country) );
        ret->Set( NODE_PSYMBOL("state"), String::New(ipreq->result.state) );
        ret->Set( NODE_PSYMBOL("city"), String::New(ipreq->result.city) );
        ret->Set( NODE_PSYMBOL("zip"), String::New(ipreq->result.zip) );
        ret->Set( NODE_PSYMBOL("latitude"), Number::New(ipreq->result.latitude) );
        ret->Set( NODE_PSYMBOL("longitude"), Number::New(ipreq->result.longitude) );
        ret->Set( NODE_PSYMBOL("areacode"), Number::New(ipreq->result.areacode) );

        Local<Value> argv[1];
        argv[0] = Local<Value>::Local(ret->ToObject());

        TryCatch try_catch;

        ipreq->cb->Call(Context::GetCurrent()->Global(), 1, argv);

        if (try_catch.HasCaught())
            FatalException(try_catch);

        ipreq->cb.Dispose();
        ipreq->vmm->Unref();
        free(ipreq->addr);
        free(ipreq);
        scope.Close(Undefined());
    }

    static Handle<Value> location(const Arguments &args) {
        HandleScope scope;
        String::Utf8Value ipaddr(args[0]->ToString());

        Local<Function> cb = Local<Function>::Cast(args[1]);
        VastMaxmind *vmm = ObjectWrap::Unwrap<VastMaxmind>(args.This());

        iprequest *request = (iprequest *)malloc(sizeof(iprequest));
        request->cb = Persistent<Function>::New(cb);
        request->vmm = vmm;
        request->addr = *ipaddr;

        //eio_custom(EIO_location, EIO_PRI_DEFAULT, EIO_locationAfter, req);
        uv_work_t* req = new uv_work_t();
        req->data = request;
        uv_queue_work(uv_default_loop(), req, locationWorker, locationAfter);

        ev_ref(EV_DEFAULT_UC);
        vmm->Ref();
        scope.Close(Undefined());

        return Undefined(); //Handle<String>(args[0]->ToString());
    }
};

Persistent<FunctionTemplate> VastMaxmind::constructor_template;

extern "C" void
init(Handle<Object> target)
{
    HandleScope scope;

    VastMaxmind::Initialize(target);
}