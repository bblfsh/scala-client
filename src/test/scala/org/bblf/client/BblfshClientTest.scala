package org.bblfsh.client

import java.io.File

import org.scalatest.FunSuite

import scala.io.Source

class BblfshClientTest extends FunSuite {

  // XXX split into several tests and better filter() tests
  test("Parse UAST for existing .java file") {
    // XXX hardcoded path
    System.load("/home/juanjux/sync/work/sourced/client-scala/src/main/scala/org/bblfsh/client/libuast/Libuast.so")

    val client = BblfshClient("0.0.0.0", 9432)
    val filename = "src/test/resources/SampleJavaFile.java" // client read it, and encode to utf-8
    assert(new File(filename).exists())
    val fileContent = Source.fromFile(filename) .getLines.mkString
    val resp = client.parse(filename, fileContent)

    // move to other tests, reading from Node
    /*
    val internalType = client.readfield(resp.uast.get, "internalType")
    val lenRoles = client.readlen(resp.uast.get, "roles")
    val internalType2 = client.InternalType(resp.uast.get)
    val token = client.Token(resp.uast.get.children(0))
    val childrenSize = client.ChildrenSize(resp.uast.get)
    val rolesSize = client.RolesSize(resp.uast.get)
    val propertiesSize = client.PropertiesSize(resp.uast.get.children(0))
    val childAt = client.ChildAt(resp.uast.get, 0)
    val propertyAt0 = client.PropertyAt(resp.uast.get.children(1), 0)
    val propertyAt1 = client.PropertyAt(resp.uast.get.children(1), 1)
    val roleAt = client.RoleAt(resp.uast.get, 0);
    */
    var filtered = client.filter(resp.uast.get, "//QualifiedName[@roleExpression]")

    assert(resp.errors.isEmpty)
    assert(resp.uast.isDefined)
    /*
    assert(internalType == "CompilationUnit")
    assert(internalType2 == internalType)
    assert(token == "package")
    assert(childrenSize == 2)
    assert(lenRoles == 1)
    assert(rolesSize == lenRoles)
    assert(childAt.internalType == "PackageDeclaration")
    assert(propertiesSize == 1)
    // gRPC load properties in an unsorted map so these could come in any order
    assert(propertyAt0 == "internalRole" || propertyAt0 == "interface")
    assert(propertyAt1 == "internalRole" || propertyAt1 == "interface")
    assert(roleAt == 34)
    */
    assert(filtered.length == 3)
  }
}
