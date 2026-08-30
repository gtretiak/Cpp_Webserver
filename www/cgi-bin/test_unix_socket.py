import socket
import sys

def test_unix_socket(path: str, is_abstract: bool = False):
	sock_address = f"\x00{path.lstrip('@')}" if is_abstract else path
	socket_type_name = "Abstract" if is_abstract else "Filesystem"

	print(f"--- Testing {socket_type_name} Socket: {path} ---")

	try:
		s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
		s.connect(sock_address)
		
		request = b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
		s.sendall(request)

		response = b""
		while True:
			chunk = s.recv(4096)
			if not chunk:
				break
			response += chunk

		print(response.decode(errors="replace"))
		s.close()
		print("Test Passed.\n")

	except Exception as e:
		print(f"Connection Failed: {e}\n")

if __name__ == "__main__":
	test_unix_socket("mytest.sock", is_abstract=True)
	test_unix_socket("/tmp/test.sock", is_abstract=False)

# curl -v --unix-socket /tmp/test.sock http://localhost/
# curl -v --abstract-unix-socket "mytest.sock" http://localhost/
