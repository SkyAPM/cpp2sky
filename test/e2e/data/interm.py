import requests

from skywalking import agent, config
from skywalking.decorators import runnable

if __name__ == '__main__':
    config.init(collector='collector:19876', service='interm')
    config.logging_level = 'DEBUG'
    
    config.flask_collect_http_params = True
    agent.start()

    from flask import Flask, Response

    app = Flask(__name__)

    @app.route("/users", methods=["POST", "GET"])
    def application():
        from skywalking.trace.context import get_context
        get_context().put_correlation("correlation", "correlation")

        res = requests.post("http://consumer:8080/pong")

        return Response(status=res.status_code)

    PORT = 8082
    app.run(host='0.0.0.0', port=PORT, debug=True)