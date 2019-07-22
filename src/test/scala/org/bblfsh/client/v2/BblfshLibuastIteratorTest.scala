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
    iter.close()
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

  "Native UAST iterator close()" should "clean up the fields" in {
    iter.hasNext() should be(true)

    iter.close()

    iter.hasNext() should be(false)
    iter.ctx should be(0)
    iter.iter should be(0)
  }

  def countNodes(root: JNode): Int = {
      var total = 0
      root match {
        case m: JObject => {
          for ((k, v) <- m) {
            total += countNodes(v)
          }
        }
        case s: JArray => {
          for (e: JNode <- s) {
            total += countNodes(e)
          }
        }
        case a: JNode => total += a.size
      }
      total
    }

  "Native UAST iterator" should "return non-empty results on decoded objects" in {
    val wholeTree = nativeRootNode.load()
    val totalJnodes = countNodes(wholeTree)
    println(s"Total number of nodes in JVM ${totalJnodes}")

    iter.hasNext() should be(true)

    val nodes = iter.toList
    nodes shouldNot be(empty)
    println(s"Iterator returned ${nodes.size} nodes")
  }

}
