import os
import cgi
import cgitb

# Enable debugging for CGI scripts (shows detailed errors in the browser)
cgitb.enable()

# Directory to save uploaded files
UPLOAD_DIR = "../images/uploads"

# Ensure the upload directory exists
os.makedirs(UPLOAD_DIR, exist_ok=True)

# print("Content-Type: text/html")
print()

try:
    # Parse the incoming form data
    form = cgi.FieldStorage()

    # Check if a file is included in the form data
    if "file" not in form or not form["file"].filename:
        print("<h1>Error: No file uploaded</h1>")
        exit()

    # Get the uploaded file
    uploaded_file = form["file"]
    filename = os.path.basename(uploaded_file.filename)

    # Save the file to the upload directory
    file_path = os.path.join(UPLOAD_DIR, filename)
    with open(file_path, "wb") as f:
        f.write(uploaded_file.file.read())

    # Respond with success
    print(f"<h1>File '{filename}' uploaded successfully!</h1>")
    print(f"<p>Saved to: {file_path}</p>")

except Exception as e:
    print("<h1>Error while processing the file</h1>")
    print(f"<pre>{e}</pre>")