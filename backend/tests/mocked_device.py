import asyncio
import websockets
import math
import time

# DEVICE_ID = "cardiacai-123"  # Use any unique ID
DEVICE_ID = "cardiacai-122"

async def send_data():
    uri = f"ws://localhost:8000/api/ws/device?device_id={DEVICE_ID}"
    async with websockets.connect(uri) as websocket:
        print(f"[Device] Connected as {DEVICE_ID}")

        start_time = time.time()
        frequency = 1  # 1 Hz sine wave frequency (adjust as needed)
        amplitude = 2500  # Amplitude of sine wave

        while True:
            elapsed = time.time() - start_time
            # Generate sine wave value between -amplitude and +amplitude
            ecg_value = amplitude * math.sin(2 * math.pi * frequency * elapsed)

            # Send as string (you can scale or offset if needed)
            await websocket.send(str(ecg_value))
            await asyncio.sleep(0.008)  # 125Hz sample rate (0.008s interval)

asyncio.run(send_data())
