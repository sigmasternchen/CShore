CC = gcc
LD = gcc
CFLAGS = -Wall -g -std=c99 -ICFloor/src/ -Ilibargo/src/ -Ilibparcival/src/ -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=500
LDFLAGS = -lpthread -lrt

CFLOOR_LIB = CFloor/libcfloor.a
LIBARGO = libargo/libargo.a
LIBPARCIVAL = libparcival/libparcival.a
LIBS = $(CFLOOR_LIB) $(LIBARGO) $(LIBPARCIVAL)

OBJS = obj/router.o obj/request.o obj/base_cfloor.o obj/base_cgi.o obj/auth.o obj/base64.o obj/common.o obj/cookies.o
DEPS = $(OBJS:%.o=%.d)

DEMO_OBJS = obj/demo.o obj/entities.tab.o obj/template.tab.o

HAS_MAIN =
NEEDS_MAIN = \
if test "$(HAS_MAIN)" != "yes"; then\
	echo "ERROR: Trying to build standalone without base.";\
	echo "       Specify target base_cfloor instead.";\
	exit 1;\
fi

all: base_cfloor clean

base_cfloor: CFLAGS += -DBASE_CFLOOR
base_cfloor: HAS_MAIN = "yes"
base_cfloor: standalone

base_cgi: CFLAGS += -DBASE_CGI
base_cgi: HAS_MAIN = "yes"
base_cgi: standalone

standalone: CFLAGS += -Idemo/
standalone: $(DEMO_OBJS) $(OBJS) $(LIBS)
	@$(NEEDS_MAIN)
	$(LD) $(LDFLAGS) -o $@ $^

$(CFLOOR_LIB):
	$(MAKE) -C CFloor/ libcfloor.a
	
$(LIBARGO):
	$(MAKE) -C libargo/ libargo.a
	
$(LIBPARCIVAL):
	$(MAKE) -C libparcival libparcival.a

-include $(DEPS)

obj/%.o: demo/%.c obj
	$(CC) $(CFLAGS) -Isrc/ -MMD -c -o $@ $<

obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -MMD -c -o $@ $<
	
obj:
	@mkdir -p obj
	
libargo/marshaller-gen:
	$(MAKE) -C libargo/ marshaller-gen
	
libparcival/parcival:
	$(MAKE) -C libparcival/ parcival
	
obj/entities.tab.o: obj/entities.tab.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

obj/entities.tab.c: demo/entities.h libargo/marshaller-gen
	./libargo/marshaller-gen -o $@ $<

obj/template.tab.o: obj/template.tab.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $<
	
obj/template.tab.c: demo/demo.templ libparcival/parcival
	cd demo && ../libparcival/parcival demo.templ > ../$@

clean:
	@echo "Cleaning up..."
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f obj/*.c
	@rm -f standalone
	$(MAKE) -C CFloor/ clean
	$(MAKE) -C libargo/ clean
	$(MAKE) -C libparcival/ clean
