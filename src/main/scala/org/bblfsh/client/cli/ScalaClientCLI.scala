package org.bblfsh.client.cli

import org.bblfsh.client.BblfshClient

import scala.io.Source

object ScalaClientCLI extends App {

  val cli = new CLI(args)
  if (args.length < 1) {
    println("Usage: [--host <bblfshServerHost> --port <bblfshServerPort>] -f <path to file>")
    cli.printHelp()
    System.exit(1)
  }
  cli.verify()

  val fileName = cli.file().getName
  val client = BblfshClient(cli.bblfshServerHost(), cli.bblfshServerPort())
  val fileContent = Source.fromFile(cli.file()).getLines.mkString("\n")
  val query = cli.query
  val resp = client.parse(fileName, fileContent)

  if (resp.errors.isEmpty) {
    if (query != None) {
      println(client.filter(resp.uast.get, query.get.get))
    } else {
      println(resp.uast.get)
    }
  } else {
    println(s"Parsing failed with ${resp.errors.length} errors:")
    resp.errors.foreach(println)
  }
}
