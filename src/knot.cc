#include <jsapi.h>
#include <uv.h>

#include <assert.h>
#include <string>
#include <fstream>
#include <sstream>

#include "knot_natives.h"

#include "tty_wrap.h"
#include "timer_wrap.h"

using knot::TTYWrap;
using knot::TimerWrap;

bool Binding(JSContext *cx, unsigned argc, JS::Value *vp) {
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


/* The class of the global object. */
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


void jsReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
    printf("%s (%s:%u)\n", message, report->filename, report->lineno);
}


static JSObject *
NewGlobalObject(JSContext *cx)
{
    JS::CompartmentOptions options;
    options.setVersion(JSVERSION_1_8);
    JS::RootedObject glob(cx, JS_NewGlobalObject(cx, &global_class, nullptr,
                                             JS::DontFireOnNewGlobalHook, options));
    if (!glob)
        return nullptr;

    {
        JSAutoCompartment ac(cx, glob);

        if (!JS_InitStandardClasses(cx, glob))
            return nullptr;

        // Setup bindings
        if (!JS_DefineFunction(cx, glob, "binding", &Binding, 1, 0))
            return nullptr;
    }

    JS_FireOnNewGlobalObject(cx, glob);

    return glob;
}


bool coreBootstrap(JSContext *cx, JS::HandleObject global)
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
        JSAutoRequest ar(cx);
        JSAutoCompartment ac(cx, global);

        CompileOptions compileOpts(cx, JSVERSION_1_8);
        compileOpts.setIntroductionType("js shell interactive")
                   .setUTF8(true)
                   .setCompileAndGo(true)
                   .setFileAndLine(bootstrapSource->name, 0);

        RootedScript script(cx);
        script = JS_CompileScript(cx, global, bootstrapSource->source, bootstrapSource->source_len, compileOpts);
        if (!script)
            return false;
        RootedValue bootstrapFunction(cx);
        if (!JS_ExecuteScript(cx, global, script, bootstrapFunction.address()))
            return false;

        RootedValue globalValue(cx, ObjectOrNullValue(global));
        HandleValueArray params(globalValue);

        RootedValue rval(cx);
        if (!JS_CallFunctionValue(cx, global, bootstrapFunction, params, &rval))
            return false;
    }

    return true;
}


bool runScript(JSContext *cx, JS::HandleObject global, const char* filename, const char* source, size_t length)
{
    using JS::CompileOptions;
    using JS::RootedScript;
    using JS::RootedValue;

    int lineno = 0;

    {
        JSAutoRequest ar(cx);
        JSAutoCompartment ac(cx, global);

        CompileOptions compileOpts(cx, JSVERSION_1_8);
        compileOpts.setFileAndLine(filename, 0);

        RootedScript script(cx);
        script = JS_CompileScript(cx, global, source, length, compileOpts);

        if (!script)
            return false;

        RootedValue scriptVal(cx);
        if (!JS_ExecuteScript(cx, global, script, scriptVal.address()))
            return false;
    }

    return true;
}


bool executeScriptFile(JSContext *cx, JS::HandleObject global, const char* filename)
{
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();

    const knot::_native* bootstrapSource = &knot::natives[0];

    std::string source = buffer.str();

    return runScript(cx, global, filename, source.c_str(), source.length());
}


uv_loop_t *setupEventLoop(JSContext *cx)
{
    uv_loop_t *loop = uv_default_loop();
    JS_SetContextPrivate(cx, loop);
    return loop;
}


int run(JSContext *cx, const char *filename)
{
    uv_loop_t *loop;

    JS_BeginRequest(cx);
    JS::RootedObject global(cx);
    global = NewGlobalObject(cx);
    if (global == NULL)
        return 1;
    JS_EndRequest(cx);

    loop = setupEventLoop(cx);

    if(!coreBootstrap(cx, global)) {
        return 2;
    }

    if (!executeScriptFile(cx, global, filename)) {
        return 3;
    }

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}


int main(int argc, const char *argv[])
{
    JSRuntime *rt;
    JSContext *cx;

    JS_Init();

    rt = JS_NewRuntime(8L * 1024 * 1024, JS_NO_HELPER_THREADS);
    if (rt == NULL)
        return 1;

    cx = JS_NewContext(rt, 8192);
    if (cx == NULL)
        return 1;

    JS_SetErrorReporter(cx, jsReporter);

    int exitCode = run(cx, argv[1]);

    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();

    return exitCode;
}
