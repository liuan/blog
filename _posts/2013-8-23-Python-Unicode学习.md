---
layout: post
category: python
tags: python
---

之前遇到这么一个问题，需要处理一个UTF-16 Litte Endian编码的文本文件，查找该文件中某个字符串，然后使用其他字符串替换该字符串。如果文本编码是普通的ASCII编码，那么就直接按照简单的查找替换思路解决问题，然而，这样的方法解决不了这个特殊编码的文件。后面就学习了下Python中的Unicode，了解下这不同编码的文件。

这儿提到的编码问题，是python2.x版本下的问题，在python3以后，编码的问题解决的很好。要理解不同编码问题，先知道basestring的子类str和unicode到底有什么区别。

在python中，str并不是一连串的字符，而是unicode经过编码之后的字节组成的序列。Unicode string 才是真正的字符序列。对UTF-8编码的str’汉’使用len()函数时，结果是3，因为实际上，UTF-8编码的’汉’ == ‘\xE6\xB1\x89’。对字节串str使用正确的字符编码进行解码之后，并且len(u’汉’) == 1。

故，对于str，可以解码。对于unicode，可以编码。

Python内置的open函数，打开文件时，read()读取的是str，读取之后需要使用正确的编码格式进行decode(),write()写入时，如果参数是unicode string，则需要使用你希望写入的编码进行encode()，如果是其他编码格式的str，则需要先对其进行解码，转换成unicode之后，然后再使用写入的编码进行encode（）。如果直接使用unicode作为参数传入write（）方法，python将先使用源代码文件声明的字符编码进行编码然后写入。

Python还存在一个模块codecs，提供了open方法，可以指定一个编码打开文件，使用这个方法打开的文件读取返回的将是unicode。写入时，如果是unicode，则按照打开时指定的编码进行编码再写入。如果是str，则先根据源代码文件中声明的编码格式，解码成unicode后在进行前述操作。

<img src="/assets/img/python_unicode01.png" width="700px">

上面这张份代码详细的描述如何了处理不同编码方式的文本。对于一个采用utf-16-le编码方式的文本，要替换其中某个字符串，采取seek，重写的方式。

* 首先读取一行，line是unicode，通过encode编码成utf-16-le,然后调用find，将str风格的searchText先解码，然后encode成utf-16-le,这样，返回的lineNum则不会出问题。

* 判断一行的长度，调用len函数时，需要将line进行编码。不然seek会出错，seek需要知道的字节数。

在附加一张科普不同编码方式基础知识的图片。

<img src="/assets/img/python_unicode02.png" width="700px">

 根据这张图片的内容，在总结一下：
> Unicode标准有UCS-2和UCS-4，分别是两个字节表示一个字符和四个字节，这只是一种字符编码的方式，类似于ASCII
> UTF(UCS Transformation Format) 则解决的字符具体如何传输和存储。

> 1.UTF-16: 直接存储UCS的编码，例如“汉”的UCS是“6C49”，UTF-16-BE，6C49，UTF-16-LE，496C
> 2.UTF-8 : 不是所有的字符都需要两个字节存储，则有了UTF-8，变长的，兼容ASCII。ASCII依然1字节保存，中文3字节保存

> BOM(Byte Order Mark): 必须记录文件到底是按什么格式存储的，才能方便的解码，UTF则引入了BOM来解决这个问题
> BOM: 文本文件的开始则插入一定的字节，如`UTF-8插入这样三个字节，ef bb bf,UTF-16-LE,ff fe,UTF-16-BE,fe ff`.
