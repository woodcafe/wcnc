from flask import Flask, render_template
from flask import request
from cnc import Cnc

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/cakes')
def cakes():
    return 'Yummy cakes!'
    
@app.route('/hello/')
@app.route('/hello/<name>')
def hello(name='world'):
    return render_template('page.html', name=name)
    
@app.route('/post/<int:post_id>')
def show_port(post_id):
    return 'Post %d' % post_id
    
@app.route('/move', methods=['POST', 'GET'])
def move():
    x=y=z=0
    if 'x' in request.args:
        x = int(request.args['x'])
    if 'y' in request.args:
        y = int(request.args['y'])
    if 'z' in request.args:
        z = int(request.args['z'])
    output = 'x=%d y=%d z=%d'% (x, y, z)
    cnc = Cnc('/dev/ttyUSB0')
    cnc.move(x, y, z)
    return output
    
#if __name__ == '__main__':
    #app.run(debug=True, host='0.0.0.0')
  