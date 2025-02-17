import cgi

# Output the headers
print("Content-Type: text/html", end="")  # Content-Type header
print("\r\n\r\n", end="")  # Blank line indicating the end of headers

# Start the HTML structure
print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head>")
print("<meta charset='UTF-8'>")
print("<meta name='viewport' content='width=device-width, initial-scale=1.0'>")
print("<title>Greeting</title>")

# Adding some CSS for the dark green theme
print("<style>")
print("body { font-family: Arial, sans-serif; background-color: #e8f5e9; color: #1b5e20; text-align: center; padding: 20px; margin: 0; }")
print("h1 { color: #388e3c; font-size: 3em; }")
print("p { font-size: 1.5em; margin-top: 20px; }")
print("footer { font-size: 1em; color: #555; margin-top: 40px; }")
print("button { font-size: 1.2em; padding: 10px 20px; background-color: #388e3c; color: white; border: none; border-radius: 5px; cursor: pointer; }")
print("button:hover { background-color: #2c6e2d; }")
print("</style>")

print("</head>")
print("<body>")

# Handle form data
form = cgi.FieldStorage()

# Retrieve values from the form
name = form.getvalue('username', 'Guest')  # Default value is 'Guest' if not provided
favfood = form.getvalue('favfood', 'Unknown')  # Default value is 'Unknown' if not provided

# Display the results in a more user-friendly way
print("<h1>Hello, " + name + "!</h1>")

# Showing length of the favorite food string
print("<p>Length of your favorite food: " + str(len(favfood)) + "</p>")

# Show a friendly message
print("<p>I've heard that your favorite food is <strong>" + favfood + "</strong>... is this true?</p>")

# Optional: Add a footer
print("<footer>CGI Script Example - Python</footer>")

# End HTML
print("</body>")
print("</html>")
