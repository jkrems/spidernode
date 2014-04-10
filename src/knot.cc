#include <assert.h>
#include <string>
#include <fstream>
#include <sstream>

#include "knot.h"

#include "knot_natives.h"

#include "tty_wrap.h"
#include "timer_wrap.h"

using knot::TTYWrap;
using knot::TimerWrap;

namespace knot {
  
static JSClass global_class = {
    "global",
    JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub,
    JS_DeletePropertyStub,
    JS_PropertyStub,
    JS_StrictPropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    nullptr
};

Knot::Knot() : runtime(0), context(0), global(0)
{}

Knot::~Knot()
{
    if (context) JS_DestroyContext(context);
    if (runtime) JS_DestroyRuntime(runtime);
    JS_ShutDown();
}

bool Knot::init(uint32_t maxBytes /* = KNOT_DEFAULT_MAX_BYTES */,
                size_t stackChunkSize /* = 8192 */)
{
    JS_Init();

    runtime = JS_NewRuntime(maxBytes, JS_NO_HELPER_THREADS);
    if (runtime == NULL)
        return false;

    context = JS_NewContext(runtime, stackChunkSize);
    if (context == NULL)
        return false;

    JS_BeginRequest(context);
    global = NewGlobalObject();
    JS_EndRequest(context);
    if (global == NULL)
        return 1;

    loop = uv_default_loop();
    if (loop == NULL)
        return 1;
    JS_SetContextPrivate(context, loop);

    if(!bootstrapCore()) {
        return 2;
    }

    return true;
}

int Knot::runScriptFile(const char *filename)
{
    JS_SetErrorReporter(context, Knot::ErrorReporter);

    return _executeScriptFile(filename);
}

int Knot::_executeScriptFile(const char *filename)
{
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();

    const knot::_native* bootstrapSource = &knot::natives[0];

    std::string source = buffer.str();

    return _executeScript(filename, source.c_str(), source.length());
}

int Knot::_executeScript(const char* filename, const char* source, size_t length)
{
    using JS::CompileOptions;
    using JS::RootedScript;
    using JS::RootedValue;
    using JS::RootedObject;

    int lineno = 0;

    {
        JSAutoRequest ar(context);
        RootedObject globRef(context, global);
        JSAutoCompartment ac(context, globRef);

        CompileOptions compileOpts(context, JSVERSION_1_8);
        compileOpts.setFileAndLine(filename, 0);

        RootedScript script(context);
        script = JS_CompileScript(context, globRef, source, length, compileOpts);

        if (!script)
            return 1;

        RootedValue scriptVal(context);
        if (!JS_ExecuteScript(context, global, script, scriptVal.address()))
            return 1;
    }

    return 0;
}

void Knot::ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
    printf("%s (%s:%u)\n", message, report->filename, report->lineno);
}

JSObject* Knot::NewGlobalObject()
{
    JS::CompartmentOptions options;
    options.setVersion(JSVERSION_1_8);
    JS::RootedObject glob(context, JS_NewGlobalObject(context, &global_class, nullptr,
                                             JS::DontFireOnNewGlobalHook, options));
    if (!glob)
        return nullptr;

    {
        JSAutoCompartment ac(context, glob);

        if (!JS_InitStandardClasses(context, glob))
            return nullptr;

        // Setup bindings
        if (!JS_DefineFunction(context, glob, "binding", &Binding, 1, 0))
            return nullptr;
    }

    JS_FireOnNewGlobalObject(context, glob);

    return glob;
}

bool Knot::Binding(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);

  JS::RootedObject exports(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));

  JSAutoByteString module(cx, args[0].toString());

  if (!strcmp(module.ptr(), "natives")) {
    const knot::_native* it = knot::natives;
    for (it = knot::natives; it->name != NULL; ++it) {
      JS::RootedValue source(cx, JS::StringValue(JS_NewStringCopyZ(cx, it->source)));
      JS_SetProperty(cx, exports,
        it->name,
        source
      );
    }
  } else if(!strcmp(module.ptr(), "tty_wrap")) {
    TTYWrap::Initialize(cx, exports);
  } else if(!strcmp(module.ptr(), "timer_wrap")) {
    TimerWrap::Initialize(cx, exports);
  } else {
    JS_ReportError(cx, "Unknown binding: %s", module.ptr());
    return false;
  }

  args.rval().setObject(*exports);
  return true;
}

bool Knot::bootstrapCore()
{
    using JS::CompileOptions;
    using JS::ObjectOrNullValue;
    using JS::RootedObject;
    using JS::RootedScript;
    using JS::RootedValue;
    using JS::HandleValueArray;
    using knot::_native;
    using knot::natives;

    const _native* bootstrapSource = &natives[0];
    int lineno = 0;

    {
        JSAutoRequest ar(context);
        RootedObject globRef(context, global);
        JSAutoCompartment ac(context, globRef);

        CompileOptions compileOpts(context, JSVERSION_1_8);
        compileOpts.setIntroductionType("js shell interactive")
                   .setUTF8(true)
                   .setCompileAndGo(true)
                   .setFileAndLine(bootstrapSource->name, 0);

        RootedScript script(context);
        script = JS_CompileScript(context, globRef, bootstrapSource->source, bootstrapSource->source_len, compileOpts);
        if (!script)
            return false;
        RootedValue bootstrapFunction(context);
        if (!JS_ExecuteScript(context, global, script, bootstrapFunction.address()))
            return false;

        RootedValue globalValue(context, ObjectOrNullValue(global));
        HandleValueArray params(globalValue);

        RootedValue rval(context);
        if (!JS_CallFunctionValue(context, globRef, bootstrapFunction, params, &rval))
            return false;
    }

    return true;
}

}

// void Binding(const FunctionCallbackInfo<Value>& args) {
//   HandleScope handle_scope(args.GetIsolate());
//   Environment* env = Environment::GetCurrent(args.GetIsolate());

//   Local<String> module = args[0]->ToString();
//   String::Utf8Value module_v(module);

//   Local<Object> cache = env->binding_cache_object();
//   Local<Object> exports;

//   if (cache->Has(module)) {
//     exports = cache->Get(module)->ToObject();
//     args.GetReturnValue().Set(exports);
//     return;
//   }

//   // Append a string to process.moduleLoadList
//   char buf[1024];
//   snprintf(buf, sizeof(buf), "Binding %s", *module_v);

//   Local<Array> modules = env->module_load_list_array();
//   uint32_t l = modules->Length();
//   modules->Set(l, OneByteString(node_isolate, buf));

//   node_module_struct* mod = get_builtin_module(*module_v);
//   if (mod != NULL) {
//     exports = Object::New();
//     // Internal bindings don't have a "module" object, only exports.
//     assert(mod->register_func == NULL);
//     assert(mod->register_context_func != NULL);
//     Local<Value> unused = Undefined(env->isolate());
//     mod->register_context_func(exports, unused, env->context());
//     cache->Set(module, exports);
//   } else if (!strcmp(*module_v, "constants")) {
//     exports = Object::New();
//     DefineConstants(exports);
//     cache->Set(module, exports);
//   } else if (!strcmp(*module_v, "natives")) {
//     exports = Object::New();
//     DefineJavaScript(exports);
//     cache->Set(module, exports);
//   } else {
//     return ThrowError("No such module");
//   }

//   args.GetReturnValue().Set(exports);
// }
