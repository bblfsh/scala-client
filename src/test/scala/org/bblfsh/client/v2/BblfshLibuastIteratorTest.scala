package org.bblfsh.client.v2

import org.bblfsh.client.v2.libuast.Libuast
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FlatSpec, Matchers}

import scala.io.Source


class BblfshLibuastIteratorTest extends FlatSpec
  with Matchers
  with BeforeAndAfter
  with BeforeAndAfterAll {

  var nativeRootNode: NodeExt = _
  var iter: Libuast.UastIterExt = _
  val mangedRootNode: JNode = JArray(
    JObject(
      "k1" -> JString("v1"),
      "k2" -> JObject(
        "k3" -> JInt(24)
      )
    ))

  override def beforeAll {
    import BblfshClient._ // enables uast.* methods

    val client = BblfshClient("localhost", 9432)
    val parse = (file: String) => {
      client.parse(file, Source.fromFile(file).getLines.mkString("\n"))
    }
    val resp = parse("src/test/resources/Tiny.java")
    nativeRootNode = resp.uast.decode.root()
    client.close()
  }

  before {
    iter = BblfshClient.iterator(nativeRootNode, BblfshClient.PreOrder)
  }

  after {
    iter.nativeDispose()
  }

  // TODO(bzz): part of upcoming UastIter impl
  // "Managed UAST iterator" should "return non-empty results on JVM objects" in {
  //   it = BblfshClient.iterator(mangedRootNode, BblfshClient.PreOrder)
  //   it.hasNext() should be(true)

  //   val nodes = it.toList
  //   nodes shouldNot be(empty)
  // }

  "Native UAST iterator init()" should "initialize the fields" in {
    iter.ctx should not be(0)
    iter.iter should not be(0)
  }

  "Native UAST iterator dispose()" should "clean up the fields" in {
    iter.nativeDispose()
    iter.ctx should be(0)
    iter.iter should be(0)
  }

  "Native UAST iterator" should "return non-empty results on decoded objects" in {
    iter.hasNext() should be(false) // FIXME(bzz): invert after hasNext impl

    val nodes = iter.toList
    nodes shouldNot be(empty)
  }

}
