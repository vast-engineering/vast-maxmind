#include <node.h>
#include <node_buffer.h>
#include <string.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

using namespace v8;
using namespace node;

class VastMaxmind : public ObjectWrap {
protected:
    static Persistent<FunctionTemplate> constructor_template;
    char *db;
    GeoIP *gi;

public:
    VastMaxmind(char *_db) : db(_db) {
        this->gi = GeoIP_open(this->db, GEOIP_MEMORY_CACHE); //GEOIP_MEMORY_CACHE | GEOIP_INDEX_CACHE);
        if (this->gi == NULL) {
            ThrowException(Exception::Error(String::New("Could not open maxmind db.")));
        }

    }
    ~VastMaxmind() {
        GeoIP_delete(this->gi);
        GeoIP_cleanup();
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
        uint ipnum;
        VastMaxmind *vmm;
        GeoIPRecord *gir;
    };

    static Handle<Value> New(const Arguments &args) {
        HandleScope scope;
        String::Utf8Value db(args[0]->ToString());

        VastMaxmind *vmm = new VastMaxmind(*db);
        vmm->Wrap(args.This());
        return args.This();
    }

    static int translateIp(char *ipaddr) {
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

    

    static void locationWorker(uv_work_t *req) {
        iprequest *ipreq = (iprequest *)(req->data);
        VastMaxmind *vmm = ipreq->vmm;


        ipreq->ipnum = translateIp(ipreq->addr);

        if (ipreq->ipnum > 0 && vmm && vmm->gi) {
            ipreq->gir = GeoIP_record_by_ipnum(vmm->gi, ipreq->ipnum);
        }
        else {
            ThrowException(Exception::Error(String::New("Maxmind db not opened.")));
        }
        
    }

    static char *copy(char *str) {
        char *strNew = new char[ strlen(str) + 1 ];
        strcpy(strNew, str);
        return strNew;
    }

    static void locationAfter(uv_work_t *req) {
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
            FatalException(try_catch);

        ipreq->cb.Dispose();
        delete ipreq;
        delete req;
    }

    static Handle<Value> location(const Arguments &args) {
        HandleScope scope;
        String::Utf8Value ipaddr(args[0]);
        Local<Function> cb = Local<Function>::Cast(args[1]);
        VastMaxmind *vmm = ObjectWrap::Unwrap<VastMaxmind>(args.Holder());
        iprequest *request = new iprequest;

        request->cb = Persistent<Function>::New(cb);
        request->vmm = vmm;
        request->addr = copy(*ipaddr);

        //eio_custom(EIO_location, EIO_PRI_DEFAULT, EIO_locationAfter, req);
        uv_work_t* req = new uv_work_t();
        req->data = request;
        uv_queue_work(uv_default_loop(), req, locationWorker, locationAfter);

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