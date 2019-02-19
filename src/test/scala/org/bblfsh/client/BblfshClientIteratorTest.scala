package org.bblfsh.client

import org.bblfsh.client.BblfshClient._

import gopkg.in.bblfsh.sdk.v1.uast.generated.Node
import gopkg.in.bblfsh.sdk.v1.uast.generated.Position
import org.scalatest.FunSuite
import org.scalatest.BeforeAndAfter

import java.io.File
import scala.io.Source

class BblfshClientIterator extends FunSuite with BeforeAndAfter {
  var rootNode: Node = _

  before {
    val child1 = new Node(internalType = "child1", 
                          startPosition = Some(Position(offset = 1)))

    val subchild21 = new Node(internalType = "subchild21", 
                              startPosition = Some(Position(offset = 6)))

    val subchild22 = new Node(internalType = "subchild22", 
                              startPosition = Some(Position(offset = 5)))

    val child2 = new Node(internalType = "child2", 
                          startPosition = Some(Position(offset = 2)),
                          children = List(subchild21, subchild22))

    rootNode = new Node(internalType = "parent", 
                        startPosition = Some(Position(offset = 0)),
                        children = List(child1, child2))
  }

  test("Use as a range") {
  }

  test("PreOrder iterator") {
    var it = BblfshClient.iterator(rootNode, BblfshClient.PreOrder)

    assert(it.hasNext())
    var n = it.next()
    assert(n.internalType == "parent")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "child1")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "child2")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "subchild21")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "subchild22")
  }

  test("PostOrder iterator") {
    var it = BblfshClient.iterator(rootNode, BblfshClient.PostOrder)

    assert(it.hasNext())
    var n = it.next()
    assert(n.internalType == "child1")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "subchild21")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "subchild22")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "child2")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "parent")
  }

  test("LevelOrder iterator") {
    var it = BblfshClient.iterator(rootNode, BblfshClient.LevelOrder)

    assert(it.hasNext())
    var n = it.next()
    assert(n.internalType == "parent")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "child1")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "child2")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "subchild21")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "subchild22")
  }

  test("PositionOrder iterator") {
    var it = BblfshClient.iterator(rootNode, BblfshClient.PositionOrder)

    assert(it.hasNext())
    var n = it.next()
    assert(n.internalType == "parent")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "child1")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "child2")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "subchild22")

    assert(it.hasNext())
    n = it.next()
    assert(n.internalType == "subchild21")

    n = it.next()
    assert(!it.hasNext())
  }
}
