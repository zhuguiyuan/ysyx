import gdb

gdb.execute("set pagination off")
gdb.execute("set confirm off")
gdb.execute("set backtrace past-main on")

seen = set()


def _function_start(pc, fallback):
    """stable address of the function containing pc:
    function entry when debug info is available, else the
    enclosing compilation unit's static block start, else pc"""
    try:
        block = gdb.block_for_pc(pc)
        if block is not None and block.function is not None:
            return int(block.function.value())
        if block is not None:
            while block.superblock is not None:
                block = block.superblock
            return int(block.start)
    except (gdb.error, TypeError, ValueError):
        return fallback
    return fallback


def frame_key(f):
    """identify a frame by (function name, stable address):
    the same function at different locations counts as one tree node"""
    pc = int(f.pc())
    name = f.name() or "??"
    return (name, _function_start(pc, pc))


def path_key():
    """current call stack = sequence of (name, addr) from top to bottom"""
    parts = []
    f = gdb.newest_frame()
    while f is not None:
        parts.append(frame_key(f))
        f = f.older()
    return tuple(parts)


def bt_if_new_path():
    """print bt only when the current call stack is a never-seen path"""
    key = path_key()
    if key not in seen:
        seen.add(key)
        print("\n===== new path: " + " <- ".join(nm for nm, _ in key) + " =====")
        gdb.execute("bt")


def step_until(pred):
    """keep stepping (s) until pred holds or the process exits"""
    while True:
        try:
            bt_if_new_path()
            if pred():
                return True
            gdb.execute("s", to_string=True)
        except Exception as e:  # noqa: BLE001
            print("\n===== end: ", e, "=====")
            return False


print("======== entry path: first instruction -> main ========")
gdb.execute("starti 10")
if step_until(lambda: gdb.newest_frame().name() == "main"):
    print("\n======== exit path: main's return -> process exit ========")
    gdb.execute("break sum.c:17")
    gdb.execute("continue")
    step_until(lambda: False)
