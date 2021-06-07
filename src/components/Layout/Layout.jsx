import axios from 'axios';
import React from 'react';
import styles from './Layout.module.scss';
import cx from 'clsx';

const Layout = () => {
    const [waterBinLevel, setWaterBinLevel] = React.useState(0);
    const [waterBowlLevel, setWaterBowlLevel] = React.useState(0);
    const [foodBinLevel, setfoodBinLevel] = React.useState(0);
    const [foodBowlLevel, setfoodBowlLevel] = React.useState(0);
    const [timeChecked, settimeChecked] = React.useState(0);
    const GET_URL = (fieldId) => `https://api.thingspeak.com/channels/1408484/fields/${fieldId}.json?api_key=4MP6THBNE2DS28XK&results=1000`;

    const calculateTime = () => {
        let year = new Date().getFullYear();
        let month = new Date().getMonth();
        let day = new Date().getDate();
        let hour = new Date().getHours();
        let minutes = new Date().getMinutes();
        let seconds = new Date().getSeconds();
        return `${day}.${month + 1}.${year}  ${hour}:${minutes}:${seconds}`;
    }

    const getDataWaterBin = () => {
        axios.get(GET_URL(1))
        .then(res => {
            const {data} = res;
            const {feeds} = data;
            const good = feeds.reverse().find(f => f.field1); 
            const waterBin = good?.field1?.replace('\r\n\r\n', '')
            setWaterBinLevel(waterBin || 0);
      })
    }

    const getDataWaterBowl = () => {
        axios.get(GET_URL(2))
        .then(res => {
            const {data} = res;
            const {feeds} = data;
            const good = feeds.reverse().find(f => f.field2); 
            const waterBowl = good?.field2?.replace('\r\n\r\n', '')
            setWaterBowlLevel(waterBowl || 0);
      })
    }

    const getDataFoodBin = () => {
        axios.get(GET_URL(3))
        .then(res => {
            const {data} = res;
            const {feeds} = data;
            const good = feeds.reverse().find(f => f.field3); 
            const foodBin = good?.field3?.replace('\r\n\r\n', '')
            setfoodBinLevel(foodBin || 0);
      })
    }

    const getDataFoodBowl = () => {
        axios.get(GET_URL(4))
        .then(res => {
            const {data} = res;
            const {feeds} = data;
            const good = feeds.reverse().find(f => f.field4); 
            const foodBowl = good?.field4?.replace('\r\n\r\n', '')
            setfoodBowlLevel(Math.floor(foodBowl|| 0));
      })
    }

    React.useEffect(() => {
        getDataWaterBin();
            getDataWaterBowl();
            getDataFoodBin();
            getDataFoodBowl();
        settimeChecked(calculateTime());
        setInterval(() => {
            getDataWaterBin();
            getDataWaterBowl();
            getDataFoodBin();
            getDataFoodBowl();
            settimeChecked(calculateTime());
        }, 5000);
      }, []);

    return (
        <div className={styles.home}>
            <div className={cx(styles.infoWrapper, styles.header)}>
                    <h3>Last checked</h3>
                    <h3 className={styles.bold}>{timeChecked}</h3>
                </div>
            <div className={styles.groupWrapper}>
                <h1 className={styles.header}>💦 Water 💦</h1>
                <div className={styles.infoWrapper}>
                    <p>Bin Level</p>
                    <p className={styles.bold}>{waterBinLevel}</p>
                </div>
                <div className={styles.infoWrapper}>
                    <p>Bowl Level</p>
                    <p className={styles.bold}>{waterBowlLevel}</p>
                </div>
                {/* <button onClick={() => handleButtonClick('water')} className={styles.button}>Add Water</button> */}
            </div>
            <div className={styles.groupWrapper}>
                <h1 className={styles.header}>🧆 Food 🧆</h1>
                <div className={styles.infoWrapper}>
                    <p>Bin Level</p>
                    <p className={styles.bold}>{foodBinLevel}</p>
                </div>
                <div className={styles.infoWrapper}>
                    <p>Bowl Level</p>
                    <p className={styles.bold}>{foodBowlLevel}</p>
                </div>
                {/* <button  onClick={() => handleButtonClick('food')} className={styles.button}>Add Food</button> */}
            </div>
        </div>
    ) 
}

export default Layout;