import axios from "axios";

const REACT_APP_API_URL = process.env.REACT_APP_API_URL;

export const saveDeviceReadings = async (
	deviceId: string,
	token: string,
	enable: boolean
) => {
	try {
		const response = await axios.post(
			`${REACT_APP_API_URL}/api/readings/save/${deviceId}?enable=${enable}`,
			{},
			{
				headers: {
					"Content-Type": "application/json",
					"Access-Control-Allow-Origin": "*",
					Authorization: `Bearer ${token}`,
				},
			}
		);

		console.log("Token Used: ", token);

		console.log("Saving device readings for token:", response);
		return response.data;
	} catch (error) {
		console.error("Error saving device readings:", error);
	}
};

export const downloadDeviceReadings = async (
	sessionId: string,
	token: string
) => {
	const response = await axios.get(
		`${REACT_APP_API_URL}/api/readings/download/${sessionId}`,
		{
			headers: {
				"Content-Type": "application/json",
				"Access-Control-Allow-Origin": "*",
				Authorization: `Bearer ${token}`,
			},
		}
	);

	return response.data;
};
