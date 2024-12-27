#!/bin/sh
docker build -t nginx-tester-img .
docker run --name nginx-tester -d -p 9090:80 nginx-tester-img