import http.client

# Open a connection to the server
conn = http.client.HTTPConnection("localhost", 8080)

# The data you want to send in chunks
chunk1 = "Hello World this is your friend"
chunk2 = "Patrik"

# Prepare the headers for chunked transfer encoding
headers = {
    'Transfer-Encoding': 'chunked',
}

# Start the HTTP request with the specified headers
conn.request("POST", "/upload", body=None, headers=headers)

# Get the connection's socket
sock = conn.sock

# Send the first chunk (size of 5, content: "Hello")
sock.sendall(f"{len(chunk1):X}\r\n".encode())  # Send chunk size in hex (uppercase 'X')
sock.sendall(chunk1.encode())  # Send chunk data
sock.sendall(b"\r\n")  # Send the chunk separator

# Send the second chunk (size of 5, content: "World")
sock.sendall(f"{len(chunk2):X}\r\n".encode())  # Send chunk size in hex (uppercase 'X')
sock.sendall(chunk2.encode())  # Send chunk data
sock.sendall(b"\r\n")  # Send the chunk separator

# Send the final chunk (size 0 to indicate the end of the request body)
sock.sendall(b"0\r\n\r\n")  # End of chunks

# Close the connection
conn.close()

print("Chunked request sent successfully")
