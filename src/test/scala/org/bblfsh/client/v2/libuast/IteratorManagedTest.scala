package org.bblfsh.client.v2.libuast

import org.bblfsh.client.v2.{BblfshClient, JArray, JInt, JNode, JObject, JString}
// TODO import org.bblfsh.client.v2.nodes._
import org.scalatest.{BeforeAndAfter, BeforeAndAfterAll, FlatSpec, Matchers}
import org.scalatest.prop.TableDrivenPropertyChecks._
import BblfshClient.{
  TreeOrder,
  PreOrder,
  PostOrder,
  ChildrenOrder,
  PositionOrder,
  AnyOrder,
  LevelOrder
}

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
    iter.close()
  }

  "Managed UAST iterator" should "return non-empty results on JVM objects" in {
    iter = BblfshClient.iterator(mangedRootNode, PreOrder)
    iter.hasNext() should be(true)

    val nodes = iter.toList

    nodes shouldNot be(empty)
    iter.hasNext() should be(false)

    nodes(0) should not be null
    nodes(0) shouldBe a[JArray]
  }

  "Managed UAST iterator" should "go though all nodes of small object" in {
    iter = BblfshClient.iterator(mangedRootNode, PreOrder)
    val nodes = iter.toList

    nodes.size should be(3) // number of composite nodes
  }

  def testTree: JObject = {
    // Helper to get a position encoded into JObject
    def encodePosition(startOffset: Int, startLine: Int, startCol: Int,
                       endOffset: Int, endLine: Int, endCol: Int): JObject = {

      JObject(
        "@type" -> JString("uast:Positions"),
        "start" -> JObject(
          "@type" -> JString("uast:Position"),
          "offset" -> JInt(startOffset),
          "line" -> JInt(startLine),
          "col" -> JInt(startCol)
        ),
        "end" -> JObject(
          "@type" -> JString("uast:Position"),
          "offset" -> JInt(endOffset),
          "line" -> JInt(endLine),
          "col" -> JInt(endCol)
        )
      )
    }

    // The actual tree
    JObject(
      "@type" -> JString("root"),
      "@pos" -> encodePosition(0,1,1, 1,1,2),
      "children" -> JArray(
        JObject(
          "@type" -> JString("son1"),
          "@pos" -> encodePosition(2,2,2, 3,2,3),
          "children" -> JArray(
            JObject(
              "@type" -> JString("son1_1"),
              "@pos" -> encodePosition(10,10,1, 12,2,2)
            ),
            JObject(
              "@type" -> JString("son1_2"),
              "@pos" -> encodePosition(10,10,1, 12,2,2)
            )
          )
        ),
        JObject(
          "@type" -> JString("son2"),
          "@pos" -> encodePosition(100,100,1,  101,100,2),
          "children" -> JArray(
            JObject(
              "@type" -> JString("son2_1"),
              "@pos" -> encodePosition(5,5,1, 6,5,2)
            ),
            JObject(
              "@type" -> JString("son2_2"),
              "@pos" -> encodePosition(15,15,1, 16,15,2)
            )
          )
        )
      )
    )
  }

  def getNodeTypes(iterator: Libuast.UastIter): List[String] =
    iterator
      .filter(_.isInstanceOf[JObject])
      .map(_ ("@type").asInstanceOf[JString].str)
      .toList

  def getNodePositions(iterator: Libuast.UastIter): List[(Long, Long, Long)] = {
    def positionToTuple(pos: JObject): Option[(Long, Long, Long)] = {
      val offset = pos.get("offset")
      val line = pos.get("line")
      val col = pos.get("col")
      val maybePos = (offset, line, col)

      maybePos match {
        case (Some(o: JInt), Some(l: JInt), Some(c: JInt)) =>
          Some(o.num, l.num, c.num)
        case _ => None
      }
    }
    
    // Filter only the nodes which correspond to
    // positions and have a start field 
    iterator
      .collect { case node: JObject =>
        node.get("@pos").collect { case pos: JObject =>
          pos.get("start").collect { case start: JObject =>
            positionToTuple(start)
          // The result of this is an Option[Option]. Convert it to a single option
          }.flatten
          // Likewise
        }.flatten
      }
      .collect { case Some(pos) => pos }
      .toList
  }

  val iterators =
    Table(
      ("order", "expected"),
      (PreOrder, Seq("root", "son1", "son1_1", "son1_2", "son2", "son2_1", "son2_2")),
      (PostOrder, Seq("son1_1", "son1_2", "son1", "son2_1", "son2_2", "son2", "root")),
      (LevelOrder, Seq("root", "son1", "son2", "son1_1", "son1_2", "son2_1", "son2_2")),
      (PositionOrder, Seq("root", "son1", "son2_1", "son1_1", "son1_2", "son2_2", "son2")),
      (AnyOrder, Seq("root", "son1", "son2_1", "son1_1", "son1_2", "son2_2", "son2")),
      (ChildrenOrder, Seq("son1", "son2"))
    )

  forAll (iterators) { (order: TreeOrder, expected: Seq[String]) =>
    val iter = BblfshClient.iterator(testTree, order)
    val nodes = getNodeTypes(iter)

    order match {
      case AnyOrder =>
        nodes.toSet shouldEqual expected.toSet
      case _ =>
        nodes shouldEqual expected
    }

    iter.close()
  }

  "Positions in PositionOrder" should "actually be ordered" in {
    val posIter = BblfshClient.iterator(testTree, PositionOrder)
    val positions = getNodePositions(posIter)
    val expected = Seq((0,1,1), (2,2,2), (5,5,1), (10,10,1), (10,10,1), (15,15,1), (100,100,1))
    positions shouldEqual expected

    posIter.close()
  }

  "Managed UAST iterator with invalid numeric order" should "use AnyOrder" in {
    val invalidNumIter = BblfshClient.iterator(testTree, -1)
    val anyOrderIter = BblfshClient.iterator(testTree, AnyOrder)
    val nodesNumIter = getNodeTypes(invalidNumIter)
    val nodesAnyIter = getNodeTypes(anyOrderIter)
    nodesNumIter shouldEqual nodesAnyIter

    invalidNumIter.close()
    anyOrderIter.close()
  }
}
