package org.bblfsh.client

import java.io.File

import org.scalatest.FunSuite

import scala.io.Source

class BblfshClientTest extends FunSuite {

  // XXX split into several tests
  test("Parse UAST for existing .java file") {
    // XXX hardcode path
    System.load("/home/juanjux/sync/work/sourced/client-scala/src/main/scala/org/bblfsh/client/libuast/Libuast.so")

    val client = BblfshClient("0.0.0.0", 9432)
    val filename = "src/test/resources/SampleJavaFile.java" // client read it, and encode to utf-8
    assert(new File(filename).exists())
    val fileContent = Source.fromFile(filename) .getLines.mkString
    val resp = client.parse(filename, fileContent)

    // move to other tests
    var internalType = client.readfield(resp.uast.get, "internalType")
    var lenRoles = client.readlen(resp.uast.get, "roles")
    var internalType2 = client.InternalType(resp.uast.get)
    var token = client.Token(resp.uast.get.children(0))
    var childrenSize = client.ChildrenSize(resp.uast.get)
    var rolesSize = client.RolesSize(resp.uast.get)
    var propertiesSize = client.PropertiesSize(resp.uast.get.children(0))
    var childAt = client.ChildAt(resp.uast.get, 0)
    var propertyAt = client.PropertyAt(resp.uast.get.children(1), 0)

    assert(resp.errors.isEmpty)
    assert(resp.uast.isDefined)
    assert(internalType == "CompilationUnit")
    assert(internalType2 == internalType)
    assert(token == "package")
    assert(childrenSize == 2)
    assert(lenRoles == 1)
    assert(rolesSize == lenRoles)
    assert(childAt.internalType == "PackageDeclaration")
    assert(propertiesSize == 1)
    assert(propertyAt == "internalRoles")
  }

}
