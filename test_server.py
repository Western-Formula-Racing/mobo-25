#!/usr/bin/env python3
"""
Simple test server to receive CAN data uploads from ESP32
Run this on your computer to test the HTTP POST functionality
"""

from flask import Flask, request, jsonify
import json
from datetime import datetime

app = Flask(__name__)

@app.route('/can', methods=['POST'])
def receive_can_data():
    try:
        data = request.get_json()
        
        print(f"\n🔄 Received CAN data at {datetime.now()}")
        print(f"📊 Number of messages: {len(data.get('messages', []))}")
        
        for i, msg in enumerate(data.get('messages', [])):
            timestamp = float(msg.get('timestamp', 0))
            dt = datetime.fromtimestamp(timestamp)
            print(f"  {i+1:2d}. ID: {msg.get('id'):>3s} | Data: {msg.get('data')} | Time: {dt}")
        
        # Validate data format
        if 'messages' not in data:
            return jsonify({'error': 'Missing messages field'}), 400
            
        for msg in data['messages']:
            if not all(key in msg for key in ['id', 'data', 'timestamp']):
                return jsonify({'error': 'Invalid message format'}), 400
                
            if len(msg['data']) != 8:
                return jsonify({'error': 'CAN data must be 8 bytes'}), 400
        
        print("✅ Data validation passed")
        return jsonify({'status': 'success', 'received': len(data['messages'])}), 200
        
    except Exception as e:
        print(f"❌ Error processing data: {e}")
        return jsonify({'error': str(e)}), 500

@app.route('/health', methods=['GET'])
def health_check():
    return jsonify({'status': 'healthy', 'timestamp': datetime.now().isoformat()})

if __name__ == '__main__':
    print("🚀 Starting test server for ESP32 CAN data uploads")
    print("📡 Listening on http://localhost:8085/can")
    print("🔍 Health check: http://localhost:8085/health")
    print("📝 Press Ctrl+C to stop")
    
    app.run(host='0.0.0.0', port=8085, debug=True)
