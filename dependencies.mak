include dependencies_klib.mak

main.o: main.c klib_string.h klib_list.h klib_defs.h klib_error.h kcrypt_ui.h mygetpass.h

kcrypt_engine.o: kcrypt_engine.c kcrypt_engine.h kcrypt_idea.h kcrypt_fast.h
kcrypt_ui.o: kcrypt_ui.c kcrypt_ui.h 
kcrypt_win32.o: kcrypt_win32.c kcrypt_win32.h kcrypt_ui.h
kcrypt_idea.o: kcrypt_idea.c kcrypt_idea.h kcrypt_engine.h
kcrypt_fast.o: kcrypt_fast.c kcrypt_fast.h kcrypt_engine.h
kcrypt_cmdui.o: kcrypt_cmdui.c kcrypt_ui.h 
mygetpass.o: mygetpass.c mygetpass.h 


