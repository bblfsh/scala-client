package org.bblfsh.client

import java.io.File

import org.scalatest.FunSuite

import scala.io.Source

class BblfshClientTest extends FunSuite {

  // XXX remove
  def getFields(o: Any): Map[String, Any] = {
    val fieldsAsPairs = for (field <- o.getClass.getDeclaredFields) yield {
      field.setAccessible(true)
      (field.getName, field.get(o)) 
    }
    Map(fieldsAsPairs :_*)
  }

  test("Parse UAST for existing .java file") {
    // XXX path
    System.load("/home/juanjux/sync/work/sourced/client-scala/src/main/scala/org/bblfsh/client/libuast/Libuast.so")

    val client = BblfshClient("0.0.0.0", 9432)
    val filename = "src/test/resources/SampleJavaFile.java" // client read it, and encode to utf-8
    assert(new File(filename).exists())
    val fileContent = Source.fromFile(filename) .getLines.mkString

    val resp = client.parse(filename, fileContent)
    // move to other tests
    val filtered: Int = client.filter(42, "whatever")
    var field = client.readfield(resp.uast.get, "internalType")
    var len = client.readlen(resp.uast.get, "roles")

    assert(resp.errors.isEmpty)
    assert(resp.uast.isDefined)
    assert(filtered == 42)
  }

}
