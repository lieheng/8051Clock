# 8051Clock  
## 杜邦线连接  
- P2.0-->JP26.DS
- P2.1-->JP26.SHCP
- P2.2-->JP26.STCP
- P2.3-->JP22.IO
- P3.2-->任意按键A
- P3.3-->任意按键B

## 操作逻辑
默认为显示模式。

显示模式下，长按B键开启/关闭整点报时功能。

显示模式下，按键A进入设置模式，设置模式下，长按键A进入设置时模式，在设置时模式下，短按键A进入设置分模式，短按键A进入设置秒模式，短按键A进入设置时模式，循环往复。

在设置时分秒模式下，短按键B使示数加一，长按键A确认更改，长按键B取消更改。
