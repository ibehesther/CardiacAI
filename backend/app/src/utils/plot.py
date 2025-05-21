import matplotlib.pyplot as plt
from io import BytesIO
from fastapi.responses import StreamingResponse

def plot_ecg_and_return_image(session: dict):
    ecg_data = session["data"]
    timestamp = session.get("timestamp", "")
    session_id = str(session.get("_id", ""))

    # X-axis: evenly spaced sample index (or timestamps if available)
    x = list(range(len(ecg_data)))
    y = ecg_data

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
    return StreamingResponse(buf, media_type="image/png", headers={
        "Content-Disposition": f"attachment; filename=ecg_session_{session_id}.png"
    })
