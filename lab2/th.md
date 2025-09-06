@obumi ➜ .../os_lab_2025/lab2/src/swap (master) $ nano swap.c
@obumi ➜ .../os_lab_2025/lab2/src/swap (master) $ gcc -o swap main.c swap.c
@obumi ➜ .../os_lab_2025/lab2/src/swap (master) $ ./swap
b a
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ nano main.c 
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ nano revert_string.c
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ gcc -o reverter main.c revert_string.c
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ ./reverter hello
Reverted: ohell
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ gcc -c revert_string.c -o revert_string.o
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ gcc -fPIC -c revert_string.c -o revert_string_pic.o
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ ar rcs librevert.a revert_string.o
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ gcc -shared -o librevert.so revert_string_pic.o
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ gcc main.c -L. -lrevert -o reverter_dynamic
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ LD_LIBRARY_PATH=. ./reverter_dynamic hello
Reverted: ohell
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ gcc  main.c ./librevert.a -o reverter_static
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ ./reverter_static 
Usage: ./reverter_static string_to_revert
@obumi ➜ .../os_lab_2025/lab2/src/revert_string (master) $ ./reverter_static hello
Reverted: ohell
