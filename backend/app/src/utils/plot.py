import matplotlib.pyplot as plt
from io import BytesIO

"""
TODO: Save the array of readings o a separate collection and
only link them to the metadata document for latent retrieval
on app first load
"""

def plot_ecg_and_return_image(session: dict):
    # We should look towards plotting only 200 points
    ecg_data = session["data"]
    timestamp = session.get("timestamp", "")
    session_id = str(session.get("_id", ""))
    MAX_POINTS = 200
    OFFSET = 50

    # X-axis: evenly spaced sample index (or timestamps if available)
    # x = list(range(len(ecg_data)))
    x = list(range(MAX_POINTS))
    if len(ecg_data) < MAX_POINTS:
        y = ecg_data
        x = x[:len(ecg_data)]
    elif len(ecg_data) < MAX_POINTS + OFFSET:
        y = ecg_data[:MAX_POINTS]
    else:
        y = ecg_data[OFFSET:MAX_POINTS+OFFSET]
    
    print(f"len x {len(x)}")
    print(f"len y {len(y)}")

    plt.figure(figsize=(12, 4))
    plt.plot(x, y, color="red")
    plt.title(f"ECG Session {session_id} ({timestamp})")
    plt.xlabel("Sample #")
    plt.ylabel("Amplitude")
    plt.grid(True)

    buf = BytesIO()
    plt.savefig(buf, format="png")
    plt.close()
    buf.seek(0)
    return buf
