FROM python:3.12-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install -r requirements.txt --no-cache-dir pika

COPY executor/ ./executor/
COPY build/bin ./bin
COPY conf/ .conf/

CMD ["python", "-u", "-m", "executor.executor"]