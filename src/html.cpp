#include "html.h"

extern std::string html_header1, html_header2, html_footer;

std::string as_html(const Code& c) {
  return "<pre><code>" + std::string(c.body) + "</code></pre>";
}

std::string as_html(const List& l) {
  std::string accum = "<ul>";
  for (auto& item : l.entries) {
    accum += "<li>" + std::string(item) + "</li>";
  }
  return accum + "</ul>";
}

std::string as_html(const OrderedList& l) {
  std::string accum = "<ol>";
  for (auto& item : l.entries) {
    accum += "<li>" + std::string(item) + "</li>";
  }
  return accum + "</ol>";
}

std::string as_html(const IdentifierDefinition& l) {
   return "TODO";
}

std::string as_html(const Table& l) {
  bool has_header = true;
  if (l.entries.size() < 3) has_header = false;
  else {
    for (auto& v : l.entries[1]) 
      if (v != "-") has_header = false;
  }
  std::string accum = "<table>";
  size_t n = 0;
  if (has_header) {
     accum += "<thead><tr>";
    for (auto& e : l.entries[n]) {
      accum += "<th>" + std::string(e) + "</th>";
    }
    accum += "</tr></thead>";
    n = 2;
  }
  accum += "<tbody>";
  for (;n < l.entries.size(); n++) {
    accum += "<tr>";
    for (auto& e : l.entries[n]) {
      accum += "<td>" + std::string(e) + "</td>";
    }
    accum += "</tr>";
  } 
  accum += "</tbody></table>";
  return accum;
}

std::string as_html(const Text& l) {
  return "<p>" + std::string(l.text) + "</p>";
}

std::string as_id(std::string_view name) {
  std::string id(name);
  size_t pos = id.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-");
  while (pos != std::string::npos) {
    if (id[pos] == '+') id[pos] = 'p';
    else id[pos] = '-';
    pos = id.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-");
  }
  return id;
}

std::string as_html_chapter(std::string name, const Chapter& ch) {
  std::string accum;
  accum += "<h" + std::to_string(ch.level) + " data-number=\"" + std::string(name) + "\" id=\"" + as_id(ch.text) + "\"><span class=\"header-section-number\">" + name + "</span> " + std::string(ch.text) + "<a href=\"#" + as_id(ch.text) + "\" class=\"self-link\"></a></h" + std::to_string(ch.level) + ">";
  for (auto& el : ch.entries) {
    accum += std::visit([](auto e){ return as_html(e); }, el);
  }

  for (size_t n = 0; n < ch.subchapters.size(); n++) {
    accum += as_html_chapter(name + "." + std::to_string(n+1), ch.subchapters[n]);
  }

  return accum;
}

std::string as_html(const Chapter& ch) {
  std::string accumulator = html_header1;
  accumulator.reserve(400000);
  accumulator += ch.text;
  accumulator += html_header2;
  accumulator += "<h1 class=\"title\" style=\"text-align:center\">" + std::string(ch.text) + "</h1>";

  for (auto& el : ch.entries) {
    accumulator += std::visit([](auto e){ return as_html(e); }, el);
  }

  size_t n = 1;
  for (auto& subch : ch.subchapters) {
    accumulator += as_html_chapter(std::to_string(n), subch);
    n++;
  }

  accumulator += html_footer;
  return accumulator;
}

std::string html_header1 = 
  "<!DOCTYPE html>\n"
  "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
  "<head>\n"
  "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
  "<meta charset=\"utf-8\">\n"
  "<meta name=\"generator\" content=\"dascandy/fiets\">\n"
  "<title>\n"

, html_header2 = 
  "</title>\n"
  "  <style type=\"text/css\">\n"
  "body {\n"
  "  margin: 5em;\n"
  "  font-family: sans-serif;\n"
  "  hyphens: auto;\n"
  "  line-height: 1.35;\n"
  "}\n"
  "ul {\n"
  "  padding-left: 2em;\n"
  "}\n"
  "h1, h2, h3, h4 {\n"
  "  position: relative;\n"
  "  line-height: 1;\n"
  "}\n"
  "a.self-link {\n"
  "  position: absolute;\n"
  "  top: 0;\n"
  "  left: calc(-1 * (3.5rem - 26px));\n"
  "  width: calc(3.5rem - 26px);\n"
  "  height: 2em;\n"
  "  text-align: center;\n"
  "  border: none;\n"
  "  transition: opacity .2s;\n"
  "  opacity: .5;\n"
  "  font-family: sans-serif;\n"
  "  font-weight: normal;\n"
  "  font-size: 83%;\n"
  "}\n"
  "a.self-link:hover { opacity: 1; }\n"
  "a.self-link::before { content: \"§\"; }\n"
  "span.identifier {\n"
  "  font-style: italic;\n"
  "}\n"
  "span.new {\n"
  "  text-decoration: underline;\n"
  "  background-color: #006e28;\n"
  "}\n"
  "span.delete {\n"
  "  text-decoration: line-through;\n"
  "  background-color: #bf0303;\n"
  "}\n"
  "p.indent {\n"
  "  margin-left: 50px;\n"
  "}\n"
  "table {\n"
  "  border: 1px solid black;\n"
  "  border-collapse: collapse;\n"
  "  margin-left: auto;\n"
  "  margin-right: auto;\n"
  "  margin-top: 0.8em;\n"
  "  text-align: left;\n"
  "  hyphens: none; \n"
  "}\n"
  "td, th {\n"
  "  padding-left: 1em;\n"
  "  padding-right: 1em;\n"
  "  vertical-align: top;\n"
  "}\n"
  "th {\n"
  "  border-bottom: 1px solid black;\n"
  "}\n"
  "</style>\n"
  "</head>\n"
  "<body>\n"
;

std::string html_footer = "</body></html>\n";


