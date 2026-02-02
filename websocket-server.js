// WebSocket Server for Remote Object Locking
// Run this with: node websocket-server.js

const WebSocket = require("ws");
const http = require("http");
const fs = require("fs");
const path = require("path");

const PORT = 8080;

// Create HTTP server for serving the remote control webpage
const server = http.createServer((req, res) => {
  if (req.url === "/" || req.url === "/remote.html") {
    // Serve the remote control interface
    fs.readFile(path.join(__dirname, "remote-control.html"), (err, data) => {
      if (err) {
        res.writeHead(404);
        res.end("File not found");
        return;
      }
      res.writeHead(200, { "Content-Type": "text/html" });
      res.end(data);
    });
  } else {
    res.writeHead(404);
    res.end("Not found");
  }
});

// Create WebSocket server
const wss = new WebSocket.Server({ server });

let clients = new Set();

wss.on("connection", (ws) => {
  console.log("📱 New client connected");
  clients.add(ws);

  ws.on("message", (message) => {
    try {
      const data = JSON.parse(message);
      console.log("📨 Received:", data);

      // Broadcast to all other clients
      clients.forEach((client) => {
        if (client !== ws && client.readyState === WebSocket.OPEN) {
          client.send(JSON.stringify(data));
        }
      });
    } catch (error) {
      console.error("❌ Message error:", error);
    }
  });

  ws.on("close", () => {
    console.log("📱 Client disconnected");
    clients.delete(ws);
  });

  ws.on("error", (error) => {
    console.error("❌ WebSocket error:", error);
  });
});

server.listen(PORT, () => {
  console.log(`🚀 WebSocket server running on port ${PORT}`);
  console.log(
    `📱 Remote control interface: http://localhost:${PORT}/remote.html`
  );
  console.log(`🔌 WebSocket URL: ws://localhost:${PORT}`);
});
