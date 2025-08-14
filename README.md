# Stanford CS 144 Networking Lab

> These labs are open to the public under the (friendly) request that, to
> preserve their value as a teaching tool, solutions not be posted publicly by anybody.

이 저장소는 Stanford University의 CS144 Computer Networking 수업에서 진행하는 실습 프로젝트를 담고 있습니다. C++로 TCP/IP 프로토콜 스택을 단계별로 구현했습니다.

## 참고 자료

* **강의 웹사이트:** [https://cs144.stanford.edu](https://cs144.stanford.edu)

## Lab sequence

* **Lab 0**: [networking warmup](docs/check0.pdf)
* **Lab 1**: [stitching substrings into a byte stream](docs/check1.pdf)
* **Lab 2**: [TCP receiver](docs/check2.pdf)
* **Lab 3**: [TCP sender](docs/check3.pdf)
* **Lab 4**: [measuring the real world](docs/check4.pdf)
* **Lab 5**: [down the stack (the network interface)](docs/check5.pdf)
* **Lab 6**: [building an IP router](docs/check6.pdf)
* **Lab 7**: [putting it all together](docs/check7.pdf)

## Set up the build system

```bash
cmake -S . -B build
```

## Compile

```bash
cmake --build build
```

## Run tests

```bash
cmake --build build --target test
```

## Run speed benchmarks

```bash
cmake --build build --target speed
```

## Lint & format

```bash
# clang-tidy 실행 (코드 개선 제안)
cmake --build build --target tidy

# 코드 포맷팅
cmake --build build --target format
```

## Requirements

* **C++17** 이상
* **CMake** 3.10 이상
* **Linux**  (Ubuntu 18.04+ 권장)
* **clang-tidy**, **clang-format**

