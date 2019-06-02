extern "C"{
void AddFunctions(llvm::orc::KaleidoscopeJIT* jit){
extern void hash__map_0int__hash();
jit->addNative("hash__map_0int__hash",hash__map_0int__hash);

extern void hash__map_0_1main();
jit->addNative("hash__map_0_1main",hash__map_0_1main);

extern void concurrent_0threading_0sleep();
jit->addNative("concurrent_0threading_0sleep",concurrent_0threading_0sleep);

extern void concurrent_0threading_0do__exit__thread();
jit->addNative("concurrent_0threading_0do__exit__thread",concurrent_0threading_0do__exit__thread);

extern void concurrent_0threading_0do__create__thread();
jit->addNative("concurrent_0threading_0do__create__thread",concurrent_0threading_0do__create__thread);

extern void concurrent_0threading_0_1main();
jit->addNative("concurrent_0threading_0_1main",concurrent_0threading_0_1main);

extern void string__buffer_0string__buffer_0to__str();
jit->addNative("string__buffer_0string__buffer_0to__str",string__buffer_0string__buffer_0to__str);

extern void string__buffer_0string__buffer_0____init____();
jit->addNative("string__buffer_0string__buffer_0____init____",string__buffer_0string__buffer_0____init____);

extern void string__buffer_0string__buffer_0append();
jit->addNative("string__buffer_0string__buffer_0append",string__buffer_0string__buffer_0append);

extern void string__buffer_0_1main();
jit->addNative("string__buffer_0_1main",string__buffer_0_1main);

extern void birdee_0pointer2str();
jit->addNative("birdee_0pointer2str",birdee_0pointer2str);

extern void birdee_0double2str();
jit->addNative("birdee_0double2str",birdee_0double2str);

extern void birdee_0breakpoint();
jit->addNative("birdee_0breakpoint",birdee_0breakpoint);

extern void birdee_0print();
jit->addNative("birdee_0print",birdee_0print);

extern void birdee_0int2str();
jit->addNative("birdee_0int2str",birdee_0int2str);

extern void birdee_0bool2str();
jit->addNative("birdee_0bool2str",birdee_0bool2str);

extern void birdee_0println();
jit->addNative("birdee_0println",birdee_0println);

extern void birdee_0____create__basic__exception__no__call();
jit->addNative("birdee_0____create__basic__exception__no__call",birdee_0____create__basic__exception__no__call);

extern void birdee_0genericarray_0length();
jit->addNative("birdee_0genericarray_0length",birdee_0genericarray_0length);

extern void birdee_0genericarray_0get__raw();
jit->addNative("birdee_0genericarray_0get__raw",birdee_0genericarray_0get__raw);

extern void birdee_0string_0length();
jit->addNative("birdee_0string_0length",birdee_0string_0length);

extern void birdee_0string_0____add____();
jit->addNative("birdee_0string_0____add____",birdee_0string_0____add____);

extern void birdee_0string_0____init____();
jit->addNative("birdee_0string_0____init____",birdee_0string_0____init____);

extern void birdee_0string_0____hash____();
jit->addNative("birdee_0string_0____hash____",birdee_0string_0____hash____);

extern void birdee_0string_0____eq____();
jit->addNative("birdee_0string_0____eq____",birdee_0string_0____eq____);

extern void birdee_0string_0____ne____();
jit->addNative("birdee_0string_0____ne____",birdee_0string_0____ne____);

extern void birdee_0string_0copy__bytes();
jit->addNative("birdee_0string_0copy__bytes",birdee_0string_0copy__bytes);

extern void birdee_0string_0get__bytes();
jit->addNative("birdee_0string_0get__bytes",birdee_0string_0get__bytes);

extern void birdee_0string_0get__raw();
jit->addNative("birdee_0string_0get__raw",birdee_0string_0get__raw);

extern void birdee_0type__info_0get__name();
jit->addNative("birdee_0type__info_0get__name",birdee_0type__info_0get__name);

extern void birdee_0type__info_0get__parent();
jit->addNative("birdee_0type__info_0get__parent",birdee_0type__info_0get__parent);

extern void birdee_0type__info_0is__parent__of();
jit->addNative("birdee_0type__info_0is__parent__of",birdee_0type__info_0is__parent__of);

extern void birdee_0_1main();
jit->addNative("birdee_0_1main",birdee_0_1main);

}}