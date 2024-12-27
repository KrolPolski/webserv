#!/bin/sh
docker build -t nginx-tester-img .
docker run --name nginx-tester -d -p 9090:80 nginx-tester-img
echo "You only need to run this script once." 
echo "You should now be able to access the nginx container's webserver at http://localhost:9090"
echo "After the first time, you can stop the nginx server by:"
echo "docker stop nginx-tester"
echo "You can start it again by running:"
echo "docker start nginx-tester"