"""This module includes a variety of unit tests to test the 42 webserv"""

import unittest
import requests
import os
import urllib.parse

class WebservResponseCodeTests(unittest.TestCase):
    """Tests if webserv returns the the appropriate response codes."""
    def setUp(self):
        self.webserv_url = 'http://localhost:8080/'
        try:
            response = requests.get(self.webserv_url, timeout=5)
        except Exception as e:
            print("Did you forget to start the webserv?")
            raise unittest.SkipTest()
    
    
    def test_basic_get(self):
        try:
            response = requests.get(self.webserv_url, timeout = 5)
        #rint(f"Basic GET test on {self.webserv_url}")
            self.assertEqual(response.status_code, 200, msg=f"Basic GET test: expected 200, received {response.status_code}")
        except Exception as e:
            raise unittest.SkipTest(f"An error occurred: {e}\n\nDid you forget to start the webserv?")
            #raise SystemExit(f"An error occurred: {e}\n\nDid you forget to start the webserv?")
    
    def test_missing_file(self):
        missing_file = self.webserv_url + "not_a_file.html"
        response = requests.get(missing_file, timeout = 5)
        #rint("GET on missing file test")
        self.assertEqual(response.status_code, 404)
    
    def test_bad_permissions(self):
        unreadable_file = 'home/cant_read.html'
        with open(unreadable_file, "w") as f:
            f.write("This is a test file.")
        os.chmod(unreadable_file, 0o000)
        test_url = self.webserv_url + "cant_read.html"
        response = requests.get(test_url, timeout = 5)
        #print("GET on file with no read permissions test")
        self.assertEqual(response.status_code, 403)
        os.chmod(unreadable_file, 0o644)  # Set to readable/writeable by owner
        os.remove(unreadable_file)  # Delete the file
    
    def test_bad_method(self):
        #test_url = self.webserv_url + "cgi/pythonPost.py"
        test_url = self.webserv_url + "index.html"
        data = {"key" : "value"}
        response = requests.post(test_url, data=data, timeout = 5)
        #print(f"Reponse: {response.status_code}")
        #print(response.text)
        self.assertEqual(response.status_code, 405)
    
    def test_good_post(self):
        test_url = self.webserv_url + "cgi/pythonPost.py"
        test_data = {"username": "Mr. Tester",
                 "favfood": "The good kind"}
        test_headers = {"Content-Type": "application/x-www-form-urlencoded"} 
        response = requests.post(test_url, data=test_data, headers=test_headers, timeout = 5)
        print(response.text)
        print(response.status_code)

    def test_delete_successful(self):
        url = "http://localhost:8080/deleteme.html"
        try:
            deletable_file = 'home/deleteme.html'
            with open(deletable_file, "w") as f:
                f.write("This is a test file.")
            os.chmod(deletable_file, 0o664)
            response = requests.delete(url, timeout = 5)
            self.assertEqual(response.status_code, 204)
        # Print the response
            print("Status Code:", response.status_code)
            print("Response Text:", response.text)
        except requests.exceptions.RequestException as e:
            print("An error occurred:", e)
    
    def test_delete_no_perms(self):
        undeletable_file = 'home/cantdelete.html'
        with open(undeletable_file, "w") as f:
            f.write("This is a test file.")
        os.chmod(undeletable_file, 0o000)
        test_url = self.webserv_url + "cantdelete.html"
        response = requests.delete(test_url, timeout = 5)
        self.assertEqual(response.status_code, 403)
        os.chmod(undeletable_file, 0o644)  # Set to readable/writeable by owner
        os.remove(undeletable_file)  # Delete the file
    
    def test_delete_non_existent(self):
        url = "http://localhost:8080/not_here.html"
        response = requests.delete(url, timeout = 5)
        self.assertEqual(response.status_code, 404)

    def test_delete_full_directory(self):
        url = "http://localhost:8080/full/"
        response = requests.delete(url, timeout = 5)
        self.assertEqual(response.status_code, 400)


if __name__ == '__main__':
    unittest.main() 