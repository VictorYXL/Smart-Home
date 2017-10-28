from flask import Flask, jsonify
from random import randint

app = Flask(__name__)

@app.route('/')
def hello_world():
    datas = [0,0,0]
    with open('config.txt') as cfg:
        content = cfg.read().replace(' ','').replace('\n','').replace('\r','')
        datas = content.split(',')
        try:
            datas = map(int ,datas)
        except:
            pass
    return jsonify({'ground_lamp':int(datas[0]), 'table_lamp':int(datas[1]), 'tv': int(datas[2]) })

if __name__ == '__main__':
    app.run("0.0.0.0", debug=True)
