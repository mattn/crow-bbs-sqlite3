#include <memory>
#include <stdexcept>
#include <sqlite3.h>
#include "crow_all.h"

int
main() {
  sqlite3 *db = nullptr;
  int r = sqlite3_open("bbs.db", &db);
  if (SQLITE_OK!=r){
    throw std::runtime_error("can't open database");
  }
  crow::SimpleApp app;
  crow::mustache::set_base(".");

  CROW_ROUTE(app, "/")
  ([&]{
    crow::mustache::context ctx;
    const char *sql = "select id, text from bbs order by created";
    sqlite3_stmt *stmt = nullptr;
    sqlite3_prepare(db, sql, -1, &stmt, nullptr);
    int n = 0;
    while (SQLITE_DONE != sqlite3_step(stmt)){
      ctx["posts"][n]["id"] = (std::string) (char*) sqlite3_column_text(stmt, 0);
      ctx["posts"][n]["text"] = (std::string) (char*) sqlite3_column_text(stmt, 1);
      n++;
    }
    sqlite3_finalize(stmt);
    return crow::mustache::load("bbs.html").render(ctx);
  });

  CROW_ROUTE(app, "/post")
      .methods("POST"_method)
  ([&](const crow::request& req, crow::response& res){
    crow::query_string params(std::string("?") + req.body);
    char* q = params.get("text");
    if (q == nullptr) {
      res = crow::response(400);
      res.write("bad request");
      res.end();
      return;
    }
    const char *sql = "insert into bbs(text) values(?)";
    sqlite3_stmt *stmt = nullptr;
    sqlite3_prepare(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, q, -1,
      (sqlite3_destructor_type) SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    res = crow::response(302);
    res.set_header("Location", "/");
    res.end();
  });

  app.port(40081)
    //.multithreaded()
    .run();
}
