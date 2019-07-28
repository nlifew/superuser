
# 引入 LOCAL_PATH
LOCAL_PATH	:=	$(call my-dir)

# 清空所有变量（除 LOCAL_PATH 外）
include $(CLEAR_VARS)

# 生成的文件名
LOCAL_MODULE	:=	su

# 编译的源文件名
LOCAL_SRC_FILES	:=	su.c


# LOCAL_LDFLAGS := -llog

# 引用系统库
# LOCAL_LDLIBS	+=	-lz

# 编译类型为可执行文件
include $(BUILD_EXECUTABLE)
