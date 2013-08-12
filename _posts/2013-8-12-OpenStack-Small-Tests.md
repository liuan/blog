---
layout: post
category: OpenStack
tags: OpenStack
---

在任何一个OpenStack Project的源码中，都有大量的测试代码，很多时间对这些测试代码都直接忽略了，觉的没什么用处，可是这次在[LaunchPad Nova](https://launchpad.net/nova)提交代码后，无法通过Jenkins的测试，尽管我在自己环境中搭建的OpenStack平台上测试通过了，被这个问题困住了，仔细看了其中的Unit Test Case发现自己没有通过测试的原因是修改了api，却没有修改测试该api的数据，所以就出错了。这些天接触了提交patch的过程，越发明白测试在开发过程中的重要性，尤其是这种开源社区协作方式，所以就特意查阅了一些测试资料！

这一篇主要介绍最基本的Small Test,我理解其实是单元测试，内容主要翻译于[OpenStack WiKi SmallTests](https://wiki.openstack.org/wiki/SmallTestingGuide)，再结合我一些我的理解。

### Small Tests

Small Tests是开发者用的最频繁的测试方法，是和源代码捆绑的最紧的测试方法了，测试粒度十分细，如果测试用例测试失败，可以迅速的定位出错的位置。以Nova为例，在nova的源码下面，有个nova/tests/目录，里面全都是各个测试用例，十分全面和细致，基本针对每个函数都有测试用例。

### 一个简单的例子

Wiki的文档中提供了一个简单的例子，引导大家来了解什么是Small Tests以及会存在哪些问题，怎样解决，就这样就将Small Tests的内容覆盖全了。

    import unittest

    TALISKER = "Talisker"
    HIGHLAND_PARK = "Highland Park"

    class OrderTests(unittest.TestCase):

        def setUp(self):
            self.warehouse = Warehouse()
            self.warehouse.add(TALISKER, 50) 
            self.warehouse.add(HIGHLAND_PARK, 25) 

        def test_order_is_filled_if_enough_in_warehouse(self):
            order = Order(TALISKER, 50) 
            order.fill(self.warehouse)
            self.assertTrue(order.is_filled())
            self.assertEqual(self.warehouse.get_inventory(TALISKER), 0)

        def test_order_does_not_remove_if_not_enough(self):
            order = Order(TALISKER, 51) 
            order.fill(self.warehouse)
            self.assertFalse(order.is_filled())
            self.assertEqual(self.warehouse.get_inventory(TALISKER), 50)

在这个例子中，就体现了Python语言中Small Test的基本框架，其实，对于熟悉xUnit-style tests的人来说，当然不管什么语言，这份代码应该很熟悉。这儿存在一个测试用例`OrderTests`继承于`unittest.TestCase`，之所以继承，这样是为了更好的适应一般的测试框架，像python-nosetests.  代码中有一个`setUp`方法用来准备初始状态，所有的测试用例都需要这样，必须实现setUp方法。每一个测试方法都是以"test"开头，这样unittests知道调用哪个方法去运行。断言则是通过使用`self.assert*()`来实现的，还有一个可选的方法`tearDown`,用来进行收场工作的，该方法发生在每个测试方法结束时。

#### 隔离

然后再我们平时做测试时，会发现自己实现的功能会依赖另一份代码，这样就引入了一个问题，测试时，不确定是自己的错误，还是另一份代码的错误。上面的代码就演示了这个小问题，`The system under test(SUT)`测试还有一个`Warehouse`，这就意味着如果warehouse出错，这很可能OrderTest也出错了，就出现刚才在前面描述的问题了。

如果我们为`Warehouse`添加充分的隔离测试，那么就在一定程度解决了这个问题,看下面的测试代码。

    class WarehouseTests(unittest.TestCase):

        def setUp(self):
            self.warehouse = Warehouse()
            self.warehouse.add('Glenlivit', 10) 

        def test_warehouse_shows_new_inventory(self):
           self.assertEqual(self.warehouse.get_inventory('Glenlivit'), 10) 

        def test_warehouse_shows_added_inventory(self):
            self.warehouse.add('Glenlivit', 15) 
            self.assertEqual(self.warehouse.get_inventory('Glenlivit'), 25) 

        def test_warehouse_shows_removed_inventory(self):
            self.warehouse.remove('Glenlivit', 10) 
            self.assertEqual(self.warehouse.get_inventory('Glenlivit'), 0)


这样如果再遇到问题，就可以推断出问题到底发生在Warehous还是Order上了。虽说这是一个使用方法，但是某种程度违背了small tests最大化隔离的原则。原文是这样`This is bending the rule of maximum isolation for small tests, but it is a parctical approach.`

### Test Doubles

上面给出的warehouse很简单，关键的是她本身没有引入其他的依赖，对于复杂的情况，到底该怎样完成隔离了？

对`the system under test(SUT)`完成隔离的基本原则是引入`Test Double`（文中举例说像a stunt double 特技替身演员）,用Test Double来替代test中的每一个依赖。`introduce a Test Double that fills in for each dependency for the purposes of that test`。有各种各样的特定类型的test double，并且有一系列的词汇来描述这些类型。Wiki上描述，为了保持内部的一致性，文档中采用Gerard Meszaros在他的`XUnit Test Patterns`一书中使用的定义。

#### Test Stub

Test Stub 就是一种Test Double，对于函数调用，它提供了一个固定的返回结果，不会考虑这个函数调用所传入的参数`(A Test Stub is a Test Double that provides a canned response to method calls)`。这也就是说，不管里怎么调用该函数，它总是返回同样的结果。当你想建立某个特定的情形来测试SUT时，这就很有用了。

    class OrderTestsWithStub(unittest.TestCase):

        def test_order_is_filled_if_enough_in_warehouse(self):

            class StubWarehouse(object):

                def get_inventory(self, item):
                    return 50

                def remove(self, item, qty):
                    pass

            warehouse = StubWarehouse()
            order = Order(TALISKER, 50)
            order.fill(warehouse)
            self.assertTrue(order.is_filled())


上面的这个例子，`StubWarehouse` 则不管输入的item，总是返回50.当然，这只是一个简单的应用，stub直接定义在test用例中，还有更加灵活的用户，在nova的tests目录文件夹下，就有很多这样的稍复杂功能更加全的stub应用。

#### Mock Object

Mock Object则采用不同的方法来确保代码的正确性。经典的方法确保SUT经过测试后的最终状态，Mock Objects则确保SUT在测试过程中的行为。Mocks能够直接编码，但是大部分时候，还是通过独立的库来创建，其中一个常用的库便是Python的 [Mox](https://code.google.com/p/pymox)。使用Mox,需要先创建一个mock object,处于‘record’模式。`The test then goes through the motions with the mock object, teaching it what to expect from the SUT and what to return. When this is complete, the mock object is placed in "replay" mode and the SUT is exercised. During the verification phase, the mock object confirms that its methods were called in order with the appropriate arguments.`这是wiki中的原文，由于对mock不是很理解，感觉翻译出来，还不如看原文，再辅以下面的代码，就明白上面说的过程了。

    # The comments in this example are included only for the sake of
    # showing the subtleties of mox--they should not be included in a
    # real test. 
    class OrderTestsWithMox(unittest.TestCase):

        def test_order_is_filled_if_enough_in_warehouse(self):
            # Create the Order as usual
            order = Order(TALISKER, 50) 

            # Create the mock warehouse object in record mode
            mocker = mox.Mox()
            warehouse = mocker.CreateMockAnything()

            # Record the sequence of actions expected from the Order object
            warehouse.get_inventory(TALISKER).AndReturn(50)
            warehouse.remove(TALISKER, 50) 

            # Put all mock objects in replay mode
            mocker.ReplayAll()

            # Exercise the Order object
            order.fill(warehouse)

            # Verify that the order is filled and that the warehouse saw
            # the correct behavior
            self.assertTrue(order.is_filled())
            mocker.VerifyAll()


行为验证带一定的风险，常常我们不关心SUT的特别的行为，我们只想确定它正确的实现了需要的接口。文章再举了一个例子说明这些话，譬如改变了Order的实现。

class Order(object):
    # ...
    def fill(self, warehouse):
	try:
            warehouse.remove(self._item, self._quantity)
            self._filled = True
        except:
            pass
    # ...

这也是一种完美有效的方式，但是上面的代码将打破上面测试用的mock object。这可能导致迷惑，需要修改test用例。原文是这样`This might create confusion and it would certainly require making modifications to the test. Because of the tendency of to overspecify the requirements of the software, it is recommended that developers avoid mock objects and behavior verification unless it is truly necessary.` 后面一句话推荐开发者避免mock objects和行为验证。

当然，还有一个好的例子，当测试cache时，最好选择行为验证，`where behavior verification is preferred is when testing a cache.A cache is specifically intended to have no noticeable difference in interface behavior than its underlying backend. In this case, behavior verification would be the simplest and best way to verify the intended functionality.`在验证cache的情况下，选择行为验证是一个最简单也是最好的方式来验证功能！ 

#### Fake Object

Fake Object提供了对象所代表的东西的一种实现。`A Fake Object provides a working implementation of object it is standing in for,`但是通常，Fake Object的简化适合测试，不适合产品。关于Fake Object,在nova/tests/目录下面有很多应用，譬如测试一个函数需要admin context的request ，则可以通过Fake Object的方式来构造一个假的admin context request来完成测试。通常，Fake Object用来代替实际从数据读取数据的过程，从数据库读取的操作对于small test来说，太慢了。看看下面的代码。

    class FakePersonGateway(object):

        def __init__(self):
            self._person_data = {}

        def insert(self, person):
            person.id = len(self._person_data)
            self._person_data[person.id] = person

        def find_by_name(self, name):
            for person in self._person_data.values():
                if person.name == name:
                    return person

        def find_by_parent(self, parent_id):
            people = []
            for person in self._person_data.values():
                if person.mother_id == parent_id or person.father_id == parent_id:
                    people.append(person)
            return people


    class FamilyTreeTests(unittest.TestCase):

        def setUp(self):
            self.gateway = FakePersonGateway()
            bob = Person('Bob Smith')
            self.gateway.insert(bob)
            alice = Person('Alice Smith')
            self.gateway.insert(alice)
            james = Person('James Smith', father_id=bob.id, mother_id=alice.id)
            self.gateway.insert(james)

        def test_child_descends_from_mother(self):
            tree = FamilyTree(self.gateway)
            self.assertTrue(tree.descends_from('Alice Smith', 'James Smith'))

        def test_father_does_not_descend_from_mother(self):
            tree = FamilyTree(self.gateway)
            self.assertFalse(tree.descends_from('Alice Smith', 'Bob Smith'))


这个例子中，SUT 是 `FamilyTree`,她依赖于`PersonGateway`来查询给定用户的孩子。因为small test 不允许与数据库交互，主要是基于效率上的考虑，所以使用了`FakePersonGateway`来代替，直接在内存的字典中操作。

### Using Test Doubles

前面介绍了Test Doubles的多种方式，每种方式都各有特点，应用在不同的场合，然而到底该怎样使用Test Doubles来代替实际的依赖了？在简单的情形下，上面提到的`Warehouse和Order`的例子就是较好的解决方案，对于复杂的例子来说，则有两种普遍的方法：依赖注入`Dependency Injection`和`Monkey Patching`。Monkey Patching这中方式在OpenStack的源码中很常见，网络的monkey patching，可以延迟载入，修改python的网路库。

#### 依赖注入

当使用依赖注入时，你写的SUT会依赖于抽象或者接口，而不是具体的实例。`You write (or refactor) the SUT in such a way that it depends on abstractions or interfaces rather than on concrete instances.` 依然上面的例子，当我们第一次测试`FamilyTree`，我们会发现她的构造函数是这样的。

    class FamilyTree(object):
        def __init__(self):
            self._person_gateway = mylibrary.dataaccess.PersonGateway()

这样FamilyTree的实现，直接依赖于具体的类`self._person_gateway = mylibrary.dataaccess.[PersonGateway]`。这种依赖导致FamilyTree很难被测试，因为创建一个PersonGateway可能需要某个配置文件和一些数据库的数据。应用依赖注入，我们可能重构FamilyTree.


    class FamilyTree(object):
        def __init__(self,person_gateway):
            self._person_gateway = person_gateway

现在，FamilyTree依赖于person_gateway,而后者是已经初始化好直接传递进来的，在这个场景下，这个变量是抽象的，可以是任何的东西，只要它实现了FamilyTree所需要的方法。我们可以方便的创建`FakePersonGateway`来注入到FamilyTree中。例如：

    person_gateway = FakePersonGateway()
    # ...
    tree = FamilyTree(person_gateway)

为了测试，使用依赖注入来重构系统有一个缺陷，当我们增加测试用例来覆盖SUT时，大概我们已经没有可以保证不破坏任何东西的small tests了。`Since we are adding tests to increase the coverage of the SUT, presumably we do not already have small tests that would ensure that the refactoring does not breaking anything.`任何没有优先级的测试覆盖的重构会更大风险的导致bug。`Any refactoring attempted without prior test coverage incurs a bigger riks of causing bugs.In this case, it may be better to ensure the SUT is thoroughly covered by Medium and Large tests before refactoring to improve Small test coverage.`

#### Monkey Patching

Moneky Patching 利用了python语言的动态性优点来修改全局的命名空间。当测试时，monkey patching 能够用来代替应test double中的硬编码的依赖。举个例子，还是上面的代码：

    class FamilyTree(object):
        def __init__(self):
            self._person_gateway = mylibrary.dataaccess.PersonGateway()

在我们的测试中，只需要简单的重写`mylibray.dataaccess.PersonGateway()`

    mylibray.dataaccess.PersonGateway = FakePersonGateway
    # ...  
    tree = FamilyTree()

有一个第三方库，[python stubout](https://code.google.com/p/pymox/source/browse/trunk/stubout.py)。当你需要撤销你设定的patch时，Stubout十分有用。这些撤销都是在`tearDown`方法中完成的。它同样为monkey patching提供了方便的方法来保留继承体系，当SUT需要检查对象的类型时这点就很有用了。

当需要为一个遗留系统添加测试时，monkey patching提供了非常有吸引力的好处来注入test double而不需要重构代码。当SUT依赖于很恶心注入的过程代码(procedural code)时，同样特别便利。

然而，基于monkey patching的测试用例通常更加脆弱，因为他们倾向于依赖SUT的实现细节。依赖monkey patches同样鼓励了不好的设计，因为她加强了这样的观念，就是SUT 完全自由的依赖于任何在全局命令空间可用的东西。鼓励依赖就会导致加紧系统的耦合度，导致系统很难重用和改变。

### 总结

这篇wiki总的来说是介绍了openstack中使用的单元测试用到的理论和实现方法，由于初次接触，理解不够，可能其中部分内容翻译不准确，有误，希望大家指出，加以改正。同时文章的前面给出了链接。这而还有一篇牛人写的博客，[介绍如何测试API的]()。

最好的方式，都参与到社区中来，这些知识就会在被迫与主动之间慢慢掌握。
    
