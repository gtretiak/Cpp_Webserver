# webserv test site

Plain HTML/JS pages to manually exercise your server from a browser: static
GET, upload (POST), download/delete, and CGI execution — including a few
CGI scripts shaped to hit each branch of `classifyCgiResponse`.

## Where to put things

Your static handler currently hardcodes root = `./www`, and `EventLoop`
special-cases exact `GET /` to read `www/html/index.html` directly (bypassing
the router). Everything else goes through the normal static handler at
`./www` + path. So:

```
<your project root>/
  www/
    html/
      index.html          <- served only for exact "/"
    style.css              <- /style.css
    upload.html             <- /upload.html
    download.html            <- /download.html
    cgi.html                <- /cgi.html
    uploads/                <- POST target dir (needs to exist)
    cgi-bin/
      echo.py
      local_redirect_to_static.py
      local_redirect_chain1.py
      local_redirect_chain2.py
      client_redirect.py
      client_redirect_with_document.py
```

Just unzip this archive so its `www/` folder sits next to wherever you run
the `webserv` binary from (or adjust paths to match your working directory).

## Permissions

The CGI scripts need the executable bit (`access(X_OK)` is checked before
`execve`). The zip preserves it, but if you extract on something that
doesn't respect unix permissions, run:

```
chmod +x www/cgi-bin/*.py
```

## Config file

You'll need a location block that matches `/cgi-bin` with a `.py`
extension configured, e.g.:

```
location /cgi-bin {
    cgi_extension .py /usr/bin/python3;
}
```

(`findCgiLocation` only checks `_has_cgi` and path prefix — it doesn't
currently use the interpreter path for execution, since `childRun` execs the
script directly and relies on its `#!/usr/bin/env python3` shebang line.)

## What each CGI script tests

- **echo.py** — plain `CGI_DOCUMENT` response. Dumps every environment
  variable it received (raw, whatever name your server used) plus the
  standard underscore-named CGI/1.1 variables side by side, so you can see
  at a glance if any naming mismatch is still there.
- **local_redirect_to_static.py** — single `Location: /upload.html` header,
  empty body → `CGI_LOCAL_REDIR` whose target is a static file (the
  "LOCAL_REDIR -> STATIC" branch in `finalizeCgi`).
- **local_redirect_chain1.py -> local_redirect_chain2.py -> echo.py** —
  two chained local redirects where each target is itself a CGI script
  (the "LOCAL_REDIR -> CGI" branch you're currently debugging).
- **client_redirect.py** — single absolute-URI `Location` header, empty
  body → `CGI_CLIENT_REDIR`.
- **client_redirect_with_document.py** — `Status` + absolute `Location` +
  `Content-Type`, non-empty body → `CGI_CLIENT_DOC_REDIR`.

## Notes on the upload page

There's no multipart/form-data parsing in `StaticRequestHandler`, so the
upload page reads the chosen file as raw bytes and POSTs them directly as
the request body (browsers add `Content-Length` automatically). POST target
must resolve to a path containing `/uploads` or `hasWDPermission` will
reject it with 403.

There's no directory listing yet, so the download page just takes a literal
path you type in — the easiest way to get a real path is to upload a file
first and read the `Location` header out of the upload result box.
