module main // 此句表示main文件，mask将从此文件开始执行

import "math" -> main // 此句将math模块中的内容渗透到main文件中
import "fmt" -> main
import "collections" -> main

func addmain(a: int, b: int) -> int {
    return a + b
}

struct point {
    x: int,
    y: int
}

interface pt impl point {
    // constructor不需返回值
    constructor(x: int, y: int) {
        x = this.x
        y = this.y
    }
    // 自定义字符串规则
    str() -> str {
        return "{{ {this.x}, {this.y} }}"
        // 如this.x = 1, this.y = 1,
        // 输出{ 1, 1 }
    }
    operator+(pt y) -> pt {
        return pt(this.x + y.x, this.y + y.y)
    }
}

// main函数
func main() {
    // print(content: str)
    var p: pt = pt(10, 10)
    print("{main.addmain(a, b)}") // 显式调用本模块（这个文件是main模块）的文件
    print("{add(a, b)}")
    var p: array<int>
    p = array<int>(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
    for i -> [0 ~ 9, 1] { // 等价于for i -> [0 ~ 9]（省略步长）
        print("{p[i]}")
    }
    p = array<int>([0 ~ 9])
    for i -> p {
        print("{p[i]}")
    }
    if true {
        print("True!")
    } else if false {
        print("This will not printed.")
    } else {
        print("And this will not printed.")
    }
    while true {
        print("Pls exit this program!") // 死循环
    }
    // return 0 // main函数无返回值，返回会报错
}
