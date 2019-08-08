package org.bblfsh.client.v2.libuast

import org.bblfsh.client.v2.{BblfshClient, JArray, JInt, JNode, JObject, JString}
// TODO import org.bblfsh.client.v2.nodes._
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FlatSpec, Matchers}

class IteratorManagedTest extends FlatSpec
  with Matchers
  with BeforeAndAfter
  with BeforeAndAfterAll {

  var iter: Libuast.UastIter = _
  val mangedRootNode: JNode = JArray(
    JObject(
      "k1" -> JString("v1"),
      "k2" -> JObject(
        "k3" -> JInt(24)
      )
    ))

  after {
    iter.nativeDispose() // FIXME: rename to .dispose()
  }

  "Managed UAST iterator" should "return non-empty results on JVM objects" in {
    iter = BblfshClient.iterator(mangedRootNode, BblfshClient.PreOrder)
    iter.hasNext() should be(true)

    val nodes = iter.toList

    nodes shouldNot be(empty)
    iter.hasNext() should be(false)

    nodes(0) should not be null
    nodes(0) shouldBe a[JArray]
  }

  "Managed UAST iterator" should "go though all nodes of small object" in {
    iter = BblfshClient.iterator(mangedRootNode, BblfshClient.PreOrder)
    val nodes = iter.toList

    nodes.size should be(3) // number of composite nodes
  }

  val pyClientTestRoot = JObject(
    "@type" -> JString("root"),
    "children" -> JArray(
      JObject(
        "@type" -> JString("son1"),
        "children" -> JArray(
          JObject("@type" -> JString("son1_1")),
          JObject("@type" -> JString("son1_2"))
        )
      ),
      JObject(
        "@type" -> JString("son2"),
        "children" -> JArray(
          JObject("@type" -> JString("son2_1")),
          JObject("@type" -> JString("son2_2"))
        )
      )
    )
  )

  def getNodeTypes(iterator: Libuast.UastIter): List[String] =
    iterator
      .filter(_.isInstanceOf[JObject])
      .map(_ ("@type").asInstanceOf[JString].str)
      .toList

  // Equivalent of the test.py#testIteratorPreOrder
  // https://github.com/bblfsh/python-client/blob/15ffb98bfa09e6aae4d1580f0e4f02eb2a530205/bblfsh/test.py#L270
  "Managed UAST iterator" should "return nodes in PreOrder" in {
    val preIter = BblfshClient.iterator(pyClientTestRoot, BblfshClient.PreOrder)
    val nodes = getNodeTypes(preIter)

    val poActual = Seq("root", "son1", "son1_1", "son1_2", "son2", "son2_1", "son2_2")
    nodes should have size (poActual.size)
    nodes shouldEqual poActual
  }

  "Managed UAST iterator" should "return nodes in PostOrder" in {
    val postIter = BblfshClient.iterator(pyClientTestRoot, BblfshClient.PostOrder)
    val nodes = getNodeTypes(postIter)

    val poActual = Seq("son1_1", "son1_2", "son1", "son2_1", "son2_2", "son2", "root")
    nodes should have size (poActual.size)
    nodes shouldEqual poActual
  }

  // TODO(#108) more tests coverage for other iteration orders, refactor to a table-driven test

}
