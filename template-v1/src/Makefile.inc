obj-y :=

#
# initops
#
obj-y += initops/initops.o

#
# lgu
#
obj-y += lgu/fio/fio_easyrw.o
obj-y += lgu/fio/fio_lock.o
obj-y += lgu/hexdump/hexdump.o
obj-y += lgu/tm/tm.o
obj-y += lgu/stdmsg/stdmsg.o

#
# threadwq
#
obj-y += threadwq/threadwq.o

#
# mempool
#
obj-y += mempool/mempool.o

obj-y += main.o

#
# cflags & ldflags
#
my-cflags-y :=
my-cflags-y += -I$(PRJ_DIR_STAGE_INC) -I$(CURDIR)
my-cflags-y += -I$(PRJ_DIR_STAGE_INC)/tirpc

my-ldflags-y :=
my-ldflags-y += -L$(PRJ_DIR_STAGE_LIB)

my-ldflags-y += -lpthread -lurcu -lurcu-cds

#;
