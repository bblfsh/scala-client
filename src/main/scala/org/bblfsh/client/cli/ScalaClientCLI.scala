package org.bblfsh.client.cli

import org.bblfsh.client.v2.BblfshClient
import org.bblfsh.client.v2.BblfshClient._

import scala.io.Source
import scala.util.control.NonFatal

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
  val query = cli.query()
  val resp = client.parse(fileName, fileContent)

  if (resp.errors.isEmpty) {
    if (query != null && query != "") {
      try {
        println(BblfshClient.filter(resp.uast.decode().root(), query))
      } catch {
        case NonFatal(e) => print(f"Filter error: $e")
      }
    } else {
      println(resp.uast.decode())
    }
  } else {
    println(s"Parsing failed with ${resp.errors.length} errors:")
    resp.errors.foreach(println)
  }
}
