# import main Flask class and request object
from flask import Flask, request, Response, abort, jsonify
from flask_cors import CORS, cross_origin
from pymongo import MongoClient
import requests

# create the Flask app
app = Flask(__name__)
CORS(app)

def get_db_connection(request):
    url_origin = request.host_url
    url = url_origin.replace('http://','').replace(':3001/','')
    client = MongoClient(url, 27017)
    db = client.iotagentjson
    return db

def send_command(request):
    request_data = request.get_json()
    command = request_data['value']
    url_origin = request.host_url
    url_ = str(url_origin).replace('3001/', '1026')
    head = {"fiware-service": "openiot", "fiware-servicepath": "/", "Content-Type": "application/json"}
    url = url_ + '/v2/entities/urn:ngsi:Fan:001/attrs?type=Fan'
    if command == 'on':
        payload = '{\"on\": {\"type\" : \"command\",\"value\" : \"1\"}}'
    elif command == 'off':
        payload = '{\"off\": {\"type\" : \"command\",\"value\" : \"1\"}}'
    else:
        payload = {}

    r = requests.patch(url, payload, headers=head)

@app.after_request
def after_request(response):
    url_origin = request.host_url
    url_ = str(url_origin).replace('3001/', '3000')
    response.headers.add('Access-Control-Allow-Origin', url_)
    response.headers.add('Access-Control-Allow-Headers', 'Content-Type,Authorization,x-api-key')
    response.headers.add('Access-Control-Allow-Methods', 'GET,PUT,POST,DELETE,OPTIONS')
    response.headers.add('Access-Control-Allow-Credentials', 'true')
    return response

@cross_origin()
@app.route('/tfmproxy', methods=['POST'])
def json_example():
    db = get_db_connection(request)
    groups = db.groups.find({})
    apikey_list = []
    for group in groups:
        apikey_list.append(group['apikey'])
    apikey = request.headers['x-api-key']
    if apikey in apikey_list:
        send_command(request)
        return "Send!"
    else:
        abort(403, description='wrong key')

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0',port=3001)