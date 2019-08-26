package org.bblfsh.client.v2.libuast

import org.bblfsh.client.v2.{BblfshClient, JArray, JNode, JObject, NodeExt}
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FlatSpec, Matchers}

import scala.io.Source


class IteratorNativeTest extends FlatSpec
  with Matchers
  with BeforeAndAfter
  with BeforeAndAfterAll {

  var nativeRootNode: NodeExt = _
  var iter: Libuast.UastIterExt = _

  override def beforeAll {
    import org.bblfsh.client.v2.BblfshClient._ // enables uast.* methods

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

  "Native UAST iterator init()" should "initialize the fields" in {
    iter.ctx should not be(0)
    iter.iter should not be(0)
  }

  "Native UAST iterator close()" should "clean up the fields" in {
    iter.hasNext() should be(true)

    iter.close()

    iter.hasNext() should be(false)
    iter.ctx should be(null)
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

    iter.hasNext() should be(true)

    val nodes = iter.toList
    nodes shouldNot be(empty)

    nodes.size should be equals (totalJnodes)
  }

}
