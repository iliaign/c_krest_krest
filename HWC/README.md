# DynamicSet

## Требования

- CMake 3.28 или новее
- Ninja 1.11 или новее
- Компилятор с поддержкой модулей C++20: GCC 14+, Clang 17+ или MSVC 19.34+

## Конфигурация

```bash
cmake -S . -B build -G Ninja
```

## Сборка

```bash
cmake --build build
```

## Запуск

```bash
./build/DynamicSet
```

Для реализации на отсортированном векторе:

```bash
./build/DynamicSet vector
```

Команды читаются из стандартного ввода.