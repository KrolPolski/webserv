
import cgi

form = cgi.FieldStorage()

name = form.getvalue('username')
favfood = form.getvalue('favfood')

print ("Hi there " + name)
print ("<br>")
print ("I've heard that your favourite food is " + favfood + "... is this true?")