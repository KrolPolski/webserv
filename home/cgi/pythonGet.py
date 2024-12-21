
import cgi

form = cgi.FieldStorage()

fname = form.getvalue('fname')
lname = form.getvalue('lname')

print ("Hello there, " + fname + " " + lname + "!")