using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using ActiveRagdoll;
using UnityEngine.UI;
public class HealthSystem : MonoBehaviour
{
    //Shader Renderer
    [SerializeField] private SkinnedMeshRenderer _bootsLow;
    [SerializeField] private SkinnedMeshRenderer _glovesLow;
    [SerializeField] private SkinnedMeshRenderer _helmetLow;
    [SerializeField] private SkinnedMeshRenderer _shoulderguardsLow;
    [SerializeField] private SkinnedMeshRenderer _skirtLow;
    //Shader materials
    private Material[] _shaderMatList;
    private Material _bootsMat;
    private Material _glovesMat;
    private Material _helmetMat;
    private Material _shoulderguardsMat;
    private Material _skirtMat;
    //Shader Property
    private string _propName = "_HitColor_On_Off";
    //Shader Damage timers
    private const float _impactDuration = 0.5f;
    private float _currImpactDuration;
    private bool _hasTakenDamage;
    private bool _hasPlayedHazardSound;
    public bool hasPlayedHazardSound { get { return _hasPlayedHazardSound; } }

    private const float _pulsePerFrame = 2.5f;
    private float _currPulseLerp;
    private bool _flipPulseLerp;

    //SFX
    [SerializeField] private bool _isFemale;
    private string[] _femaleFalls = new string[2] { "event:/Master/Character/Damage/death screams rodluvan-001", "event:/Master/Character/Damage/death screams rodluvan-002" };
    private string[] _maleTakeDmg = new string[5] { "event:/Master/Character/Grunt", "event:/Master/Character/Short Pain Grunt", "event:/Master/Character/Low voice damage sound", "event:/Master/Character/Pain Puke", "event:/Master/Character/Damage Sound" };
    private string[] _femaleTakeDmg = new string[4] { "event:/Master/Character/Damage/attack grunts rodluvan-001", "event:/Master/Character/Damage/attack grunts rodluvan-002", "event:/Master/Character/Damage/attack grunts rodluvan-003", "event:/Master/Character/Damage/attack grunts rodluvan-004" };
    private string[] _bloodSounds = new string[3] { "event:/Master/Character/Damage/blood 1", "event:/Master/Character/Damage/blood 2", "event:/Master/Character/Damage/blood 3"};


    //Rumble
    [SerializeField] private Rumble _rumbleScript;

    //Damage VFX's
    [SerializeField] private ParticleSystem _bloodSmall;
    [SerializeField] private ParticleSystem _bloodSplash;
    [SerializeField] private ParticleSystem _bloodCloud;

    [SerializeField] private ParticleSystem _bonkPoof;
    [SerializeField] private ParticleSystem _bonk;

    [SerializeField] private ParticleSystem _electric;

    public ParticleSystem Electric { get { return _electric; } }
    //Respawn VFX
    public ParticleSystem respawnVFX;

    //PlayerRing
    public GameObject PlayerRing;

    //HP
    private Animator _damageUIAnim = null;
    public Animator DamageUIAnim { get { return _damageUIAnim; } set { _damageUIAnim = value; } } //Set by GameHandler
    private Image _healthBar;
    public Image HealthBar { get { return _healthBar; } set { _healthBar = value; } }  //Set by GameHandler
    [SerializeField] private float _currHealth;
    [SerializeField] private int _maxHealth;

    //Script References
    [SerializeField] private PlayerMovementBehaviour _playerMovementBehaviour;
    [SerializeField] private ActiveRagdoll.ActiveRagdoll _activeRagdoll;
    [SerializeField] private PlayerAnimationBehaviour _playerAnimationBehavior;

    private MultipleTargetCamera MTCamera;

    private int _lastHitByID = -1;
    public int LastHitByID { get { return _lastHitByID; } set { _lastHitByID = value; } }
    public int MaxHealth { get { return _maxHealth; } set { _maxHealth = value; } }
    public float CurrHealth { get { return _currHealth; } set { _currHealth = value; } }
    private bool _alive = true;
    public bool alive { get { return _alive; } }
    private bool _shouldRespawn;
    private float _currRespawnTime;
    public float currRespawnTime { get { return _currRespawnTime; } set { _currRespawnTime = value; } }
    private float _respawnTime;
    public float respawnTime { get { return _respawnTime; } }
    public bool ShouldRespawn { get { return _shouldRespawn; } }

    private bool _hasInstaSpawn;
    public bool hasInstaSpawn { get { return _hasInstaSpawn; } set { _hasInstaSpawn = value; } }

    private void Awake()
    {
        _respawnTime = GameHandler.instance.RespawnTimer;

        _bootsMat = _bootsLow.material;
        _glovesMat = _glovesLow.material;
        _helmetMat = _helmetLow.material;
        _shoulderguardsMat = _shoulderguardsLow.material;
        _skirtMat = _skirtLow.material;
        _shaderMatList = new Material[] { _bootsMat, _glovesMat, _helmetMat, _shoulderguardsMat, _skirtMat };

    }

    private void Update()
    {
        MTCamera = FindObjectOfType(typeof(MultipleTargetCamera)) as MultipleTargetCamera;
        if (!MTCamera)
            return;

        if(_hasTakenDamage) //Shader Impact
        {
            if (_currImpactDuration >= _impactDuration)
            {
                //ShaderUpdate
                foreach (Material material in _shaderMatList)
                {
                    material.SetFloat(_propName, 0);
                }
                _hasTakenDamage = false;
                _currImpactDuration = 0;
            }
            _currImpactDuration += Time.deltaTime;
        }

        if (_currHealth <= _maxHealth/2 & _alive) //Shader pulse
        {
            Pulse();
        }


        if (_currHealth > 0)
        {
            return;
        }
        if (_alive) //if health is 0
        {
            _rumbleScript.LongRumble();
            _playerMovementBehaviour.InterruptPlayerState(PlayerStates.DEAD);
            MTCamera.RemoveFromList(this.transform);
            _alive = false;
            GetComponent<PlayerInput>().DeactivateInput();
            _playerMovementBehaviour.DestroyWeapon(); //Destroy weapon if this had one

            if (!_electric.isPlaying)
            {
                _activeRagdoll.Kill();
            }
            PlayerRing.SetActive(false); //Turn off playerring

            if (_lastHitByID != -1)
            {
                PlayerSetup.instance.PlayerList[_lastHitByID].GetComponent<PlayerController>().Points++; //Add one point to the last player that did damage to this
                GameHandler.instance.PlayerContainerToUse.transform.GetChild(_lastHitByID).gameObject.GetComponent<PlayerContainerPoints>().UpdatePointText();//Update killcount in UI for Player that killed this
            }
        }
        else if (!_shouldRespawn)
        {
            _currRespawnTime += Time.deltaTime;

            if (_currRespawnTime >= _respawnTime)
            {
                MTCamera.AddToList(this.transform);
                _shouldRespawn = true;
                if (_electric.isPlaying || _playerAnimationBehavior.animationAnimator.GetCurrentAnimatorStateInfo(0).IsName("Electrocution"))
                {
                    _playerAnimationBehavior.ToggleElectrocutionAnimation(false);
                    _electric.Stop();
                }

                if (_playerMovementBehaviour.GrabbedByPlayerID != -1) //If you're grabbed while dead and about to respawn
                    PlayerSetup.instance.PlayerList[_playerMovementBehaviour.GrabbedByPlayerID].GetComponent<PlayerMovementBehaviour>().InterruptPlayerState(PlayerStates.GRABBED);

                _playerMovementBehaviour.PlayerState = PlayerStates.MOVEMENT;
                
            }
        }

    }

    ///////////////////////////////////////////////////////////////////////////
    public void Damage(int damageAmount, int playerIndex = -1, GameObject hazard = null)
    {
        _hasTakenDamage = true;
        _currHealth -= damageAmount;

        _healthBar.fillAmount = _currHealth / _maxHealth; //update fillamount for HP bar
        _damageUIAnim.SetTrigger("GotHit"); //Play HitAnimation in UI
        if (_currHealth < 0)
        {
            _currHealth = 0;
        }
        if (playerIndex != -1 && playerIndex != _lastHitByID)
        {
            _lastHitByID = playerIndex;
        }

        //ShaderUpdate
        foreach (Material material in _shaderMatList)
        {
            material.SetFloat(_propName, 1);
        }

        _rumbleScript.ShortRumble();

        if (hazard && hazard.GetComponent<HazardInfo>() && !_hasPlayedHazardSound)
        {
            _hasPlayedHazardSound = true;

            switch (hazard.GetComponent<HazardInfo>().HazardType)
            {
                case HazardTypes.WATER:
                    if(_isFemale)
                    {
                        int randNr = Random.Range(0, _femaleTakeDmg.Length);
                        FMODUnity.RuntimeManager.PlayOneShot(_femaleTakeDmg[randNr], transform.position);
                    }
                    else
                    {
                        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Pain Puke", transform.position);
                    }
                    FMODUnity.RuntimeManager.PlayOneShot("event:/Master/SFX/BRS_Water_Splash_Lunge_11", transform.position);
                    break;
                case HazardTypes.ELECTRIC:
                    if (_isFemale)
                    {
                        int randNrElec = Random.Range(0, _femaleFalls.Length);
                        FMODUnity.RuntimeManager.PlayOneShot(_femaleFalls[randNrElec], transform.position);
                    }
                    else
                    {
                        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Grunt 2", transform.position);
                    }

                    FMODUnity.RuntimeManager.PlayOneShot("event:/Master/SFX/PSE_EFV1_FX_One_Shot_Foley_Electric_Welding - Trimmed", transform.position);
                    _electric.Play();
                    break;
                case HazardTypes.SPIKE:
                    if (_isFemale)
                    {
                        int randNrSpike = Random.Range(0, _femaleTakeDmg.Length);
                        FMODUnity.RuntimeManager.PlayOneShot(_femaleTakeDmg[randNrSpike], transform.position);
                    }
                    else
                    {
                        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Grunt 2", transform.position);
                    }

                    FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Damage/blood Splat", transform.position);
                    int randNrBlood = Random.Range(0, _bloodSounds.Length);
                    FMODUnity.RuntimeManager.PlayOneShot(_bloodSounds[randNrBlood], transform.position);

                    _bloodSmall.Play();
                    _bloodSplash.Play();
                    _bloodCloud.Play();
                    break;
                case HazardTypes.SAW:
                    if (_isFemale)
                    {
                        int randNrSaw = Random.Range(0, _femaleTakeDmg.Length);
                        FMODUnity.RuntimeManager.PlayOneShot(_femaleTakeDmg[randNrSaw], transform.position);
                    }
                    else
                    {
                        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Grunt 2", transform.position);
                    }
                    FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Damage/blood Splat", transform.position);
                    int randNrBloodSaw = Random.Range(0, _bloodSounds.Length);

                    FMODUnity.RuntimeManager.PlayOneShot(_bloodSounds[randNrBloodSaw], transform.position);
                    _bloodSmall.Play();
                    _bloodSplash.Play();
                    _bloodCloud.Play();
                    break;
                case HazardTypes.TRAIN:
                    if (_isFemale)
                    {
                        int randNrTrain = Random.Range(0, _femaleFalls.Length);
                        FMODUnity.RuntimeManager.PlayOneShot(_femaleFalls[randNrTrain], transform.position);
                    }
                    else
                    {
                        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Scream Longest", transform.position);
                    }

                    FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Damage/blood Splat", transform.position);
                    int randNrBloodTrain = Random.Range(0, _bloodSounds.Length);
                    FMODUnity.RuntimeManager.PlayOneShot(_bloodSounds[randNrBloodTrain], transform.position);

                    _bloodSmall.Play();
                    _bloodSplash.Play();
                    _bloodCloud.Play();
                    break;
                case HazardTypes.FALL:
                    if (_isFemale)
                    {
                        int randNrFall = Random.Range(0, _femaleFalls.Length);
                        FMODUnity.RuntimeManager.PlayOneShot(_femaleFalls[randNrFall], transform.position);
                    }
                    else
                    {
                        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Scream Longest", transform.position);
                    }
                    break;
            }

            return;
        }

        if (hazard) //If we died to a hazard with no specified type, like a deep killbox for example
            return;

        if (damageAmount != 0 && _alive) //if player got hit from a Damaging weapon
        {
            if (_isFemale)
            {
                if (_currHealth == 0)
                {
                    FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Death breath Female", transform.position);
                }
                else
                {
                    int randNr = Random.Range(0, _femaleTakeDmg.Length);
                    FMODUnity.RuntimeManager.PlayOneShot(_femaleTakeDmg[randNr], transform.position);
                }
            }
            else
            {
                if (_currHealth == 0)
                {
                    FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Death breath", transform.position);
                }
                else
                {
                    int randNr = Random.Range(0, _maleTakeDmg.Length);
                    FMODUnity.RuntimeManager.PlayOneShot(_maleTakeDmg[randNr], transform.position);
                }
            }

            FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Character/Damage/blood Splat", transform.position);
            _bloodSmall.Play();
            _bloodSplash.Play();
            _bloodCloud.Play();

        }
        else if (_alive) //if player got hit from a non-damaging weapon
        {
            if (_isFemale)
            {
                int randNr = Random.Range(0, _femaleTakeDmg.Length);
                FMODUnity.RuntimeManager.PlayOneShot(_femaleTakeDmg[randNr], transform.position);
            }
            else
            {
                int randNr = Random.Range(0, _maleTakeDmg.Length);
                FMODUnity.RuntimeManager.PlayOneShot(_maleTakeDmg[randNr], transform.position);
            }

            _bonk.Play();
            _bonkPoof.Play();

        }
    }

    public void ResetHealthVariables()
    {
        if (!_alive)
            _activeRagdoll.Alive();

        if (_healthBar)
            _healthBar.fillAmount = _maxHealth;
        _currHealth = _maxHealth;
        _alive = true;
        _shouldRespawn = false;
        _currRespawnTime = 0;
        _lastHitByID = -1;
        
        PlayerRing.SetActive(true);
        _hasPlayedHazardSound = false;
        foreach (Material material in _shaderMatList)
        {
            material.SetFloat(_propName, 0);
        }
        _hasInstaSpawn = false;

    }
    public float GetHealth()
    {
        return _currHealth;
    }

    private void Pulse()
    {
        if (_currPulseLerp < 0.8f && !_flipPulseLerp)
        {
            _currPulseLerp = Mathf.LerpUnclamped(_currPulseLerp, 1, _pulsePerFrame * Time.deltaTime);
            foreach (Material material in _shaderMatList)
            {
                material.SetFloat(_propName, _currPulseLerp);
            }
            return;
        }
        else if (!_flipPulseLerp)
        {
            _flipPulseLerp = true;
        }
        /////////////////////////////////
        if (_currPulseLerp > 0 && _flipPulseLerp)
        {
            _currPulseLerp = Mathf.LerpUnclamped(_currPulseLerp, -1, _pulsePerFrame * Time.deltaTime);
            foreach (Material material in _shaderMatList)
            {
                material.SetFloat(_propName, _currPulseLerp);
            }
            return;
        }
        else if (_flipPulseLerp)
        {
            _flipPulseLerp = !_flipPulseLerp;
        }
    }
}
