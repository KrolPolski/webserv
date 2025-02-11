
import cgi

form = cgi.FieldStorage()

name = form.getvalue('username')
favfood = form.getvalue('favfood')

print ("Length of favfood is: \n")
print (len(favfood))

print ("Hi there " + name)
print ("<br>")
print ("I've heard that your favourite food is " + favfood + "... is this true?")