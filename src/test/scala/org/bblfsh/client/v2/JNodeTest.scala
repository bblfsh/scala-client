package org.bblfsh.client.v2

import org.scalatest.{BeforeAndAfter, FlatSpec, Matchers}


class JNodeTest extends FlatSpec
  with BeforeAndAfter
  with Matchers {

  val rootTree = JArray(
    JObject(
      "k1" -> JString("v1"),
      "k2" -> JBool(false)
    ),
    JString("test")
  )

  "JNode" should "expose children" in {
    rootTree.children.size shouldEqual 2
    rootTree.children(1).children shouldBe empty
  }

  "JNode" should "expose size" in {
    rootTree.size shouldEqual rootTree.children.size

    val obj = rootTree.children(0)
    obj.size shouldEqual obj.children.size

    val str = rootTree.children(1)
    str.size shouldEqual str.asInstanceOf[JString].str.length
  }

  "JNode" should "expose valueAt" in {
    val valueAtEqualsChildren = (node: JNode) => {
      for (i <- 0 to node.size - 1) {
        node.valueAt(i) shouldEqual node.children(i)
      }
    }

    val arr = rootTree
    valueAtEqualsChildren(arr)

    val obj = rootTree.children(0)
    valueAtEqualsChildren(obj)
  }

  "JNode" should "expose get by key" in {
    val obj = rootTree.children(0)
    obj("k1").getClass shouldBe classOf[JString]
    obj("k1") shouldBe JString("v1") // TODO(bzz) add implicit conversions

    obj("k2").getClass shouldBe classOf[JBool]
    obj("k2") shouldBe JBool(false)
  }

  "JNode object and array" should "expose add" in {
    // object
    val obj = rootTree.children(0).asInstanceOf[JObject]
    var sizeBeforeAdd = obj.size
    obj.add("b", JNull())

    obj.size shouldEqual sizeBeforeAdd + 1

    // array
    val arr = rootTree
    sizeBeforeAdd = arr.size
    arr.add(JNull())

    arr.size shouldEqual sizeBeforeAdd + 1
  }

}
