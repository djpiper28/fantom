import subprocess
import os
import sys

NO_LEAKS = "All heap blocks were freed -- no leaks are possible"
TEST_EXEC_NAME = "fantom-tests"
VALGRIND_OPTS = "--leak-check=full --show-leak-kinds=all --track-fds=yes --fair-sched=yes"  # all" silly ubuntu has no all


def tests():
    print(
        f"Running memcheck for {TEST_EXEC_NAME}",
    )

    p = subprocess.Popen(
        f"valgrind {VALGRIND_OPTS} ./{TEST_EXEC_NAME}",
        shell=True,
        bufsize=4096,
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdout=subprocess.DEVNULL,
    )

    output = ""
    for line in p.stderr:
        nl = "\n"
        print(f">>{line.decode('UTF-8').split(nl)[0]}")
        output += line.decode("UTF-8")
    p.wait()

    if NO_LEAKS in output and p.returncode == 0:
        return 0
    else:
        return 1


def main():
    a = tests()
    if a == 1:
        print("Tests failed")
        sys.exit(1)

    sys.exit(0)


if __name__ == "__main__":
    main()
