from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import ssl


class RequestHandler(BaseHTTPRequestHandler):

    def do_POST(self):
        # TEST: Fault injection: HTTP 500 #################################
        #if self.path == "/api/error500":
        #    body = json.dumps({
        #    "status": "error",
        #    "message": "intentional test error"
        #    }).encode("utf-8")
        #
        #    self.send_response(500)
        #    self.send_header("Content-Type", "application/json")
        #    self.send_header("Content-Length", str(len(body)))
        #    self.end_headers()
        #
        #    self.wfile.write(body)
        #    return
        ####################################################################


        if self.path != "/api/data":
            self.send_response(404)
            self.end_headers()
            return

        content_length = int(
            self.headers.get("Content-Length", 0)
        )

        body = self.rfile.read(content_length)

        try:
            data = json.loads(body)

            print("Received JSON:")
            print(json.dumps(data, indent=4))

        except json.JSONDecodeError:
            self.send_response(400)
            self.end_headers()
            return


        response = {
            "status": "ok",
            "message": "data received via HTTPS"
        }

        response_body = json.dumps(
            response
        ).encode("utf-8")


        self.send_response(200)

        self.send_header(
            "Content-Type",
            "application/json"
        )

        self.send_header(
            "Content-Length",
            str(len(response_body))
        )

        self.end_headers()

        self.wfile.write(response_body)


server = HTTPServer(
    ("0.0.0.0", 8443),
    RequestHandler
)


tls_context = ssl.SSLContext(
    ssl.PROTOCOL_TLS_SERVER
)


tls_context.load_cert_chain(
    certfile="server_cert.pem",
    keyfile="server_key.pem"
)


server.socket = tls_context.wrap_socket(
    server.socket,
    server_side=True
)


print(
    "HTTPS server started on port 8443"
)


server.serve_forever()