#include <jsapi.h>
#include <uv.h>

#include <assert.h>
#include <string>
#include <fstream>
#include <sstream>

#include "knot_natives.h"

#include "tty_wrap.h"

using knot::TTYWrap;

bool Binding(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  // JSObject *global = JS_GetGlobalForObject(cx, &args.callee());

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
        JS_ConvertStub
};


void jsReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
    printf("%s (%s:%u)\n", message, report->filename, report->lineno);
}


int main(int argc, const char *argv[])
{
    JSRuntime *rt;
    JSContext *cx;
    JS::RootedObject  *global;

    uv_loop_t *loop = uv_default_loop();

    JS_Init();

    rt = JS_NewRuntime(8L * 1024 * 1024, JS_NO_HELPER_THREADS);
    if (rt == NULL)
        return 1;

    cx = JS_NewContext(rt, 8192);
    if (cx == NULL)
        return 1;

    global = new JS::RootedObject(cx, JS_NewGlobalObject(cx, &global_class, nullptr, JS::DontFireOnNewGlobalHook));
    if (global == NULL)
        return 1;

    JS_SetErrorReporter(cx, jsReporter);
    JS::CompileOptions *mainOptions = new JS::CompileOptions(cx, JSVERSION_1_8);
    JS::CompileOptions bsOptions(cx, JSVERSION_1_8);

    JS::Value rval;
    JSString *str;
    bool ok;

    const char *filename = argv[1];
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();

    const knot::_native* bootstrapSource = &knot::natives[0];

    std::string script = buffer.str();

    int lineno = 0;
    {
        JSAutoCompartment ac(cx, *global);
        JS_InitStandardClasses(cx, *global);

        // Setup bindings
        if (!JS_DefineFunction(cx, *global, "binding", &Binding, 1, 0))
          return 1;

        // Eval.
        bsOptions.setIntroductionType("js shell interactive")
               .setUTF8(true)
               .setCompileAndGo(true)
               .setFileAndLine(bootstrapSource->name, lineno);
        JS::RootedScript bsScript(cx);
        bsScript = JS_CompileScript(cx, *global, bootstrapSource->source, bootstrapSource->source_len, bsOptions);
        if (!bsScript)
            return 1;
        JS::RootedValue bsResult(cx);
        if (!JS_ExecuteScript(cx, *global, bsScript, bsResult.address()))
            return 1;

        JS::RootedValue globalValue(cx, ObjectOrNullValue(*global));
        JS::HandleValueArray bsParams(globalValue);

        JS::RootedValue bsFnResult(cx);
        if (!JS_CallFunctionValue(cx, *global, bsResult, bsParams, &bsFnResult))
            return 1;

        JSScript* compiled = JS_CompileScript(cx, *global, script.c_str(), script.length(), mainOptions->setFileAndLine(filename, lineno));

        if (!compiled)
            return 1;

        ok = JS_ExecuteScript(cx, *global, compiled, &rval);
        if (!ok || (rval.isNull() | rval.isFalse() ) )
            return 1;
    }

    uv_run(loop, UV_RUN_DEFAULT);
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();

    return 0;
}
