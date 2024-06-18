## 简介

​	`CRC`即循环冗余校验码`(Cyclic Redundancy Check)`：是数据通信领域中最常用的一种查错校验码，其特征是信息字段和校验字段的长度可以任意选定。循环冗余检查`CRC`是一种数据传输检错功能，对数据进行多项式计算，并将得到的结果附在帧的后面，接收设备也执行类似的算法，以保证数据传输的正确性和完整性

​	项目基于Qt6.7版本开发

### **CRC算法参数模型解释**

`NAME`：参数模型名称
`WIDTH`：宽度，即`CRC`比特数
`POLY`：生成项的简写，以`16`进制表示。例如：`CRC-32`即是`0x04C11DB7`，忽略了最高位的`"1"`，即完整的生成项是`0x104C11DB7`
`INIT`：这是算法开始时寄存器`crc`的初始化预置值，十六进制表示
`REFIN`：待测数据的每个字节是否按位反转，`True`或`False`
`REFOUT`：在计算后之后，异或输出之前，整个数据是否按位反转，`True`或`False`
`XOROUT`：计算结果与此参数异或后得到最终的`CRC`值



## 发布

| [已发布][release-link] | [下载][download-link] |      下载次数      |
| :--------------------: | :-------------------: | :----------------: |
|    ![release-badge]    |   ![download-badge]   | ![download-latest] |

[release-link]: https://github.com/jackfahdin/Qcrc/releases "Release status"
[release-badge]: https://img.shields.io/github/release/jackfahdin/Qcrc.svg?style=flat-square "Release status"
[download-link]: https://github.com/jackfahdin/Qcrc/releases/latest "Download status"
[download-badge]: https://img.shields.io/github/downloads/jackfahdin/Qcrc/total.svg "Download status"
[download-latest]: https://img.shields.io/github/downloads/jackfahdin/Qcrc/latest/total.svg "latest status"



